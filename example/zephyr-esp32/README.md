# Zephyr ESP32 OTA Example

This example shows how to use the Keyhan OTA agent on an ESP32 running Zephyr,
using the Zephyr OSAL implementation (`g_keyhan_zephyr_osal_ops`).

## Expected manifest format

The server response body for `GET /v1/manifest` must be a binary frame:

- Header (6 bytes):
  - `version` (1 byte)
  - `msg_len` (4 bytes, body length)
  - `msg_type` (1 byte, `1` = manifest)
- Body (variable, max `KEYHAN_OTA_PROTOCOL_MAX_BODY_LEN`)
  - `target_version` (int32)
  - `image_size` (uint32)
  - `image_url_len` (uint16)
  - `image_url` bytes (not null-terminated)
- Footer (4 bytes)
  - `0xDEADBEEF`

## Build (example)

```bash
west build -b esp32 example/zephyr-esp32
```

Use your actual board target (for example `esp32_devkitc_wroom`).
