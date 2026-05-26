# Keyhan

Keyhan is a small, portable C library that runs over-the-air (OTA) firmware updates on embedded devices. It checks a manifest server for newer firmware, downloads the image, and applies it through a platform-specific layer (OSAL).

## Documentation

| Document | Audience | Description |
|----------|----------|-------------|
| [User Guide](docs/user-guide.md) | Integrators and firmware engineers | Concepts, configuration, server setup, and step-by-step integration |
| [Developer Guide](docs/developer-guide.md) | Contributors and port authors | Architecture, APIs, protocol wire format, build system, and porting |

## Quick start (ESP-IDF)

```bash
cd example/esp32-sync-http
idf.py set-target esp32
idf.py build flash monitor
```

Configure Wi‑Fi credentials, manifest URL, and device token in `example/esp32-sync-http/main/main.c` before building.

See the [User Guide](docs/user-guide.md) for full setup, manifest format, and operational details.

## Repository layout

```
include/keyhan/     Public headers (agent, protocol, osal, error)
src/core/           Platform-neutral agent and protocol logic
src/port/           ESP-IDF and Zephyr OSAL implementations
ports/esp32/keyhan/ ESP-IDF component registration
example/            Reference applications (ESP-IDF, Zephyr)
docs/               User and developer documentation
```

## Examples

- **ESP-IDF (recommended):** `example/esp32-sync-http` — uses the current agent API and built-in ESP-IDF HTTP OSAL.
- **Zephyr:** `example/zephyr-esp32` — demonstrates a custom HTTP stack with Zephyr sockets; see the example README for manifest format notes.
