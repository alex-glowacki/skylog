# SkyLog

A solar-powered, ESP32-based weather station with local data logging, MQTT telemetry,
and a Grafana dashboard. Deployed in Hillsboro, ND (Traill County).

## Architecture
Sensors → ESP32 (edge node) → Wi-Fi → MQTT (Mosquitto)
↓
Telegraf
↓
InfluxDB v2
↓
Grafana

## Repository Layout

| Directory | Contents |
|---|---|
| `firmware/skylog-node/` | ESP32 PlatformIO project (C++) |
| `server/skylog-server/` | Python MQTT subscriber, data pipeline, REST API |
| `dashboard/grafana/` | Grafana provisioning files |
| `docs/` | Wiring diagrams, BOM, calibration notes, architecture |

## Quick Start

### Firmware

```bash
cd firmware/skylog-node
pio run                      # compile
pio run --target upload      # flash to ESP32
pio device monitor           # open serial monitor at 115200 baud
```

### Server

```bash
cd server/skylog-server
pip install -e ".[dev]"
pytest
```

## Hardware

- **MCU:** ESP32
- **Sensors:** Arduino Nicla Sense Env (HS4001, ZMOD4510, ZMOD4410, LPS22HB)
- **Wind:** Davis Instruments 6410 anemometer + wind vane
- **Power:** 6V monocrystalline solar panel → CN3791 MPPT → 3.7V 2500mAh LiPo

## License

MIT