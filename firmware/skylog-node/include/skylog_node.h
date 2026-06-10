/**
 * @file skylog_node.h
 * @brief Shared declarations and constants for the SkyLog ESP32 edge node.
 */

#pragma once

#include <stdint.h>

// ── Firmware version
// ──────────────────────────────────────────────────────────
#define SKYLOG_FW_VERSION "0.1.0"

// ── Timing
// ────────────────────────────────────────────────────────────────────
/** Deep-sleep duration between sensor readings (microseconds). */
static constexpr uint64_t SLEEP_DURATION_US = 60ULL * 1000000ULL; // 60 s

/** Maximum time (ms) to wait for a Wi-Fi connection before giving up. */
static constexpr uint32_t WIFI_TIMEOUT_MS = 15000U;

/** Maximum time (ms) to wait for MQTT broker connection. */
static constexpr uint32_t MQTT_TIMEOUT_MS = 10000U;

// ── MQTT
// ──────────────────────────────────────────────────────────────────────
#define MQTT_ROOT_TOPIC "skylog/node1/"
#define MQTT_CLIENT_ID "skylog-node1"

// ── I²C ──────────────────────────────────────────────────────────────────────
/** Default ESP32 I²C pins (override in platformio.ini build_flags if needed).
 */
#ifndef PIN_SDA
#define PIN_SDA 21
#endif
#ifndef PIN_SCL
#define PIN_SCL 22
#endif

// ── ADC / battery
// ─────────────────────────────────────────────────────────────
/** GPIO connected to the battery voltage divider. */
#define PIN_BATT_ADC 34

/** Voltage divider ratio: R1=100kΩ, R2=100kΩ → factor = 2.0 */
static constexpr float BATT_DIVIDER_FACTOR = 2.0f;

/** ESP32 ADC reference voltage (mV). */
static constexpr float ADC_VREF_MV = 3300.0f;

/** ADC resolution (12-bit). */
static constexpr float ADC_RESOLUTION = 4095.0f;

// ── Sensor reading struct
// ─────────────────────────────────────────────────────
struct SensorReading {
  float temperature_c;   ///< Degrees Celsius (HS4001)
  float humidity_pct;    ///< Relative humidity % (HS4001)
  float pressure_hpa;    ///< Barometric pressure hPa (LPS22HB)
  float no2_ppb;         ///< NO₂ concentration ppb (ZMOD4510)
  float o3_ppb;          ///< O₃ concentration ppb (ZMOD4510)
  float wind_speed_mps;  ///< Wind speed m/s (Davis 6410)
  uint16_t wind_dir_deg; ///< Wind direction degrees 0–359 (Davis 6410)
  uint32_t battery_mv;   ///< Battery voltage mV
  int8_t rssi_dbm;       ///< Wi-Fi RSSI dBm
  uint32_t uptime_s;     ///< Seconds since last hard reset
};