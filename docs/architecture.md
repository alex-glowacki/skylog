# SkyLog — Architecture

## Data Flow
Sensors (Nicla Sense Env, Davis 6410)
│
▼
ESP32 (skylog-node)

FreeRTOS tasks: sensor read, MQTT publish, OTA
Deep sleep 60 s between cycles
NVS: persists calibration offsets across reboots
│ Wi-Fi / MQTT
▼
Mosquitto (MQTT broker — Proxmox LXC)
│
▼
Telegraf (ingestion agent — subscribes to skylog/#)
│
▼
InfluxDB v2 (time-series DB — Proxmox LXC)
│
▼
Grafana (dashboards + alerting — Proxmox LXC)


## Sensor → Topic Mapping

| Sensor | MQTT Topic |
|---|---|
| HS4001 temperature | `skylog/node1/temperature` |
| HS4001 humidity | `skylog/node1/humidity` |
| LPS22HB pressure | `skylog/node1/pressure` |
| ZMOD4510 NO₂ | `skylog/node1/air_quality/no2` |
| ZMOD4510 O₃ | `skylog/node1/air_quality/o3` |
| Davis 6410 speed | `skylog/node1/wind_speed` |
| Davis 6410 direction | `skylog/node1/wind_direction` |
| Battery (ADC) | `skylog/node1/status/battery_mv` |
| Wi-Fi RSSI | `skylog/node1/status/rssi` |
| Uptime | `skylog/node1/status/uptime` |

## Power Budget

| Component | Avg draw |
|---|---|
| ESP32 (active + Wi-Fi ~2 s/min) | ~8 mA |
| Nicla Sense Env | ~10 mA |
| Davis 6410 (reed switch, passive) | ~0 mA |
| **Total estimated** | **~15–20 mA** |

2500 mAh LiPo @ 15 mA = ~165 hours backup without sun.