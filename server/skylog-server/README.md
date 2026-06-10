# skylog-server

Python backend for the SkyLog weather station. Subscribes to the MQTT broker,
validates and enriches incoming sensor data, and exposes a REST API for Grafana.

## Installation

```bash
pip install -e ".[dev]"
```

## Usage

```bash
skylog-server
```

## Development

```bash
pytest --cov=src
ruff check src tests
```