/**
 * @file main.cpp
 * @brief SkyLog ESP32 edge node — entry point.
 *
 * Responsibilities (each in its own FreeRTOS task):
 *   - Read sensors (Nicla Sense Env over I²C, Davis 6410 pulse count)
 *   - Publish readings to MQTT broker
 *   - Handle OTA firmware updates
 *   - Enter deep sleep between cycles
 *
 * @version 0.1.0
 */

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <esp_log.h>
#include <esp_sleep.h>

#include "skylog_node.h"

// ─ Credentials (define in platformio.ini build_flags or a local secrets
// header) ─
#ifndef WIFI_SSID
#error                                                                         \
    "WIFI_SSID is not defined. Add -DWIFI_SSID='\"your_ssid\"' to build_flags."
#endif
#ifndef WIFI_PASS
#error                                                                         \
    "WIFI_PASS is not defined. Add -DWIFI_PASS='\"your_password\"' to build_flags."
#endif
#ifndef MQTT_HOST
#error                                                                         \
    "MQTT_HOST is not defined. Add -DMQTT_HOST='\"broker_ip\"' to build_flags."
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif

static const char *TAG = "skylog";

// ── Globals ─────────────────────────────────────────────────────────────────
static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);
static SensorReading reading = {};

// ── Forward declarations ────────────────────────────────────────────────────
static bool connectWiFi();
static bool connectMQTT();
static bool readSensors(SensorReading &out);
static void publishReadings(const SensorReading &r);
static void enterDeepSleep();

// ────────────────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  ESP_LOGI(TAG, "SkyLog node v%s booting", SKYLOG_FW_VERSION);

  if (!connectWiFi()) {
    ESP_LOGE(TAG, "Wi-Fi failed - entering deep sleep");
    enterDeepSleep();
  }

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setKeepAlive(30);

  if (!connectMQTT()) {
    ESP_LOGE(TAG, "MQTT failed - entering deep sleep");
    enterDeepSleep();
  }

  if (!readSensors(reading)) {
    ESP_LOGE(TAG, "Sensor read failed - publishing fault and sleeping");
    // TODO: publish a fault status message before sleeping
  }

  publishReadings(reading);

  mqttClient.disconnect();
  WiFi.disconnect(true);

  enterDeepSleep();
}

void loop() {
  // Intentionally empty.
  // All logic runs once in setup() then mode enters deep sleep.
}

// ── Wi-Fi ───────────────────────────────────────────────────────────────────

static bool connectWiFi() {
  ESP_LOGI(TAG, "Connecting to Wi-Fi SSID: %s, WIFI_SSID");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - start) >= WIFI_TIMEOUT_MS) {
      ESP_LOGE(TAG, "Wi-Fi timeout after %u ms", WIFI_TIMEOUT_MS);
      return false;
    }
    delay(250);
  }

  reading.rssi_dbm = static_cast<int8_t>(WiFi.RSSI());
  ESP_LOGI(TAG, "Wi-Fi connected. IP: %s  RSSI: %d dBm",
           WiFi.localIP().toString().c_str(), reading.rssi_dbm);
  return true;
}

// ── MQTT ────────────────────────────────────────────────────────────────────

static bool connectMQTT() {
  ESP_LOGI(TAG, "Connecting to MQTT broker at %s:%d", MQTT_HOST, MQTT_PORT);

  const uint32_t start = millis();
  while (!mqttClient.connected()) {
    if ((millis() - start) >= MQTT_TIMEOUT_MS) {
      ESP_LOGE(TAG, "MQTT timeout. State: %d", mqttClient.state());
      return false;
    }
    mqttClient.connect(MQTT_CLIENT_ID);
    delay(500);
  }

  ESP_LOGI(TAG, "MQTT connected");
  return true;
}

// ── Sensors ─────────────────────────────────────────────────────────────────

static bool readSensors(SensorReading &out) {
  // TODO: initialize Wire and Nicla Sense Env library
  // TODO: read HS4001 → out.temperature_c, out.humidity_pct
  // TODO: read LPS22HB → out.pressure_hpa
  // TODO: read ZMOD4510 → out.no2_ppb, out.o3_ppb
  // TODO: read Davis 6410 pulse count → out.wind_speed_mps, out.wind_dir_deg

  // Battery voltage via ADC
  const int raw = analogRead(PIN_BATT_ADC);
  out.battery_mv =
      static_cast<uint32_t>((static_cast<float>(raw) / ADC_RESOLUTION) *
                            ADC_VREF_MV * BATT_DIVIDER_FACTOR);

  out.uptime_s = millis() / 1000U;

  ESP_LOGI(TAG, "Battery: %u mV   Uptime: %u s", out.battery_mv, out.uptime_s);
  return true; // Replace with false on sensor init failure
}

// ── MQTT publish ────────────────────────────────────────────────────────────

static void publishReadings(const SensorReading &r) {
  char topic[64];
  char payload[32];

  auto pub = [&](const char *subtopic, float value, int decimals = 2) {
    snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "%s", subtopic);
    snprintf(payload, sizeof(payload), "%.*f", decimals, value);
    mqttClient.publish(topic, payload, /*retain=*/true);
    ESP_LOGD(TAG, "PUB %s → %s", topic, payload);
  };

  pub("temperature", r.temperature_c);
  pub("humidity", r.humidity_pct);
  pub("pressure", r.pressure_hpa);
  pub("wind_speed", r.wind_speed_mps);
  pub("wind_direction", r.wind_dir_deg);
  pub("air_quality/no2", r.no2_ppb);
  pub("air_quality/o3", r.o3_ppb);

  // Status updates
  snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "status/battery_mv");
  snprintf(payload, sizeof(payload), "%lu", r.battery_mv);
  mqttClient.publish(topic, payload, true);

  snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "status/rssi");
  snprintf(payload, sizeof(payload), "%d", r.rssi_dbm);
  mqttClient.publish(topic, payload, true);

  snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "status/uptime");
  snprintf(payload, sizeof(payload), "%lu", r.uptime_s);
  mqttClient.publish(topic, payload, true);
}

// ── Deep sleep ──────────────────────────────────────────────────────────────

static void enterDeepSleep() {
  ESP_LOGI(TAG, "Entering deep sleep for %llu s",
           SLEEP_DURATION_US / 1000000ULL);
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_US);
  esp_deep_sleep_start();
}