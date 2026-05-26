# Keyhan User Guide

This guide explains how to use Keyhan on an embedded device: what it does, what you need on the server side, how to configure the agent, and how to run the included examples.

---

## Table of contents

1. [What Keyhan does](#what-keyhan-does)
2. [Prerequisites](#prerequisites)
3. [Core concepts](#core-concepts)
4. [Update flow](#update-flow)
5. [Server requirements](#server-requirements)
6. [Integrating Keyhan on your device](#integrating-keyhan-on-your-device)
7. [Configuration reference](#configuration-reference)
8. [Events and callbacks](#events-and-callbacks)
9. [Operational modes](#operational-modes)
10. [Running the ESP-IDF example](#running-the-esp-idf-example)
11. [Troubleshooting](#troubleshooting)
12. [Glossary](#glossary)

---

## What Keyhan does

Keyhan is an **OTA update agent** for resource-constrained devices. On each update cycle it:

1. Contacts your **manifest endpoint** with a device token.
2. Parses a **binary manifest** that describes the target firmware version, image size, and download URL.
3. **Downloads** the firmware image over HTTP.
4. **Stages** and **applies** the update through platform code (flash partition, NVS version counter, reboot, and so on).

Your application chooses how often to run updates (for example, once at boot or every few minutes in a loop).

---

## Prerequisites

| Requirement | Notes |
|-------------|--------|
| **C toolchain** | ESP-IDF 5.x for ESP32, or Zephyr SDK for Zephyr targets |
| **Network** | Device must reach the manifest and image hosts (HTTP in current ports) |
| **OTA server** | HTTP server that returns the binary manifest format (see [Server requirements](#server-requirements)) |
| **Device token** | Shared secret sent as `X-Device-Token` on manifest requests |
| **Current version** | Integer your firmware stores (NVS, flash, etc.); used to skip already-installed builds |

---

## Core concepts

### Agent

The **agent** is the main object you create with `keyhan_agent_init()`. It holds configuration, state, and a pointer to platform OTA operations.

### Manifest URL

HTTP(S) URL the device calls to ask “is there an update?”. The response body must be a **binary frame**, not JSON (despite example URLs that say `.json`).

### Device token

Opaque string authenticated on the server via the header:

```http
X-Device-Token: <your-device-token>
```

### Current version

Monotonic integer representing the firmware build on the device. If the manifest’s `target_version` is not greater than `current_version`, the agent reports **running** and does not download.

### OSAL (platform layer)

**Operating System Abstraction Layer** — three functions your platform provides (or that the built-in ESP-IDF port provides):

- Fetch and parse manifest → `keyhan_agent_update_t`
- Download image and stage bytes
- Apply staged image (and optionally reboot)

On ESP-IDF, `g_keyhan_osal_ops` is linked automatically when you depend on the `keyhan` component.

---

## Update flow

```mermaid
sequenceDiagram
    participant App as Your application
    participant Agent as Keyhan agent
    participant OSAL as Platform OSAL
    participant Server as OTA server

    App->>Agent: keyhan_agent_init(config)
    App->>Agent: keyhan_agent_start() or keyhan_agent_step()
    Agent->>OSAL: fetch_update_info(manifest_url, token)
    OSAL->>Server: GET manifest (X-Device-Token)
    Server-->>OSAL: binary manifest frame
    OSAL-->>Agent: version, size, image_url

    alt No newer version
        Agent-->>App: KEYHAN_AGENT_OK (state: RUNNING)
    else Newer version available
        Agent->>OSAL: download_and_stage(info)
        OSAL->>Server: GET image_url
        Server-->>OSAL: firmware bytes
        opt auto_apply == false
            Agent-->>App: on_update_ready()
        else auto_apply == true
            Agent->>OSAL: apply_staged_update(version, auto_reboot)
            OSAL-->>App: reboot or commit version
        end
    end
```

Typical application pattern:

1. Initialize networking and persistent storage (version number).
2. Fill `keyhan_agent_config_t` and call `keyhan_agent_init()`.
3. Call `keyhan_agent_start()` once, then `keyhan_agent_step()` periodically (or only when you want a check).

---

## Server requirements

### Manifest request

| Item | Value |
|------|--------|
| Method | `GET` |
| Header | `X-Device-Token: <token>` |
| Accept | `application/octet-stream` (recommended) |
| Response body | Binary manifest frame (see [Developer Guide — Protocol](developer-guide.md#ota-protocol-v1)) |

### Manifest response layout (summary)

The body is **not JSON**. Layout:

| Section | Size | Content |
|---------|------|---------|
| Header | 6 bytes | `version` (1), `msg_len` (4 LE), `msg_type` (1, must be `1`) |
| Body | `msg_len` bytes | `target_version` (4), `image_size` (4), `image_url_len` (2), `image_url` (variable) |
| Footer | 4 bytes | `0xEF BE AD DE` on the wire (little-endian `0xDEADBEEF`) |

Constraints enforced by the device:

- Protocol `version` must be `1`.
- `image_url_len` must be &gt; 0 and &lt; 256.
- Total body length ≤ 512 bytes.
- If `image_size` is non-zero, downloaded bytes must match exactly.

### Firmware image request

The device performs `GET` on the URL embedded in the manifest (for example `http://192.168.1.100:8080/firmware.bin`). No custom headers are required for the image download in the ESP-IDF port.

### Server-side logic (recommended)

1. Validate `X-Device-Token`.
2. Compare device-reported version (you may track per device) with latest release.
3. If up to date, return HTTP 204 or an empty body — **note:** the current ESP-IDF OSAL treats empty or invalid bodies as errors; for “no update” you can return a manifest with `target_version` ≤ device version, or extend the OSAL to map HTTP 204 to `KEYHAN_AGENT_ERR_NO_UPDATE`.
4. If an update exists, build the binary frame with correct `image_size` and a reachable `image_url`.

---

## Integrating Keyhan on your device

### ESP-IDF

1. Add the Keyhan component to your project:

   ```cmake
   # In your project CMakeLists.txt
   set(EXTRA_COMPONENT_DIRS path/to/keyhan/ports/esp32/keyhan)
   ```

   Or copy the pattern from `example/esp32-sync-http/CMakeLists.txt`.

2. Declare a dependency in your app component:

   ```cmake
   idf_component_register(
       SRCS "main.c"
       REQUIRES keyhan nvs_flash esp_wifi ...
   )
   ```

3. Include headers and wire the agent:

   ```c
   #include "keyhan/agent.h"
   #include "keyhan/error.h"

   keyhan_agent_t *agent = NULL;
   keyhan_agent_config_t cfg = {
       .manifest_url = "http://192.168.1.100:8080/manifest",
       .device_token = "device-001",
       .current_version = app_version_from_nvs,
       .auto_apply = true,
       .auto_reboot = false,
       .events = { .on_progress = my_progress, .on_error = my_error },
   };

   keyhan_agent_init(&agent, &cfg);
   keyhan_agent_start(agent);   /* runs one step immediately */

   /* In your task loop: */
   keyhan_agent_step(agent);
   ```

4. Persist `current_version` after a successful apply (the example uses NVS key `app_version`).

### Custom staging (production firmware)

The ESP-IDF port downloads into memory and validates size; it does **not** write to the OTA partition by default. For production:

- Implement `download_and_stage` to write chunks to `esp_ota_write()`, or
- Provide `keyhan_staging_t`-style callbacks in a custom OSAL override.

See the [Developer Guide](developer-guide.md) for the OSAL contract.

---

## Configuration reference

`keyhan_agent_config_t` fields:

| Field | Type | Description |
|-------|------|-------------|
| `manifest_url` | `const char *` | Full URL for manifest `GET` |
| `device_token` | `const char *` | Sent as `X-Device-Token` |
| `current_version` | `int` | Local build number |
| `auto_apply` | `bool` | If true, call `apply_staged_update` after download; if false, fire `on_update_ready` and stop |
| `auto_reboot` | `bool` | Passed to OSAL apply (platform may restart immediately) |
| `osal_ops_override` | `const keyhan_osal_ota_ops_t *` | Optional; `NULL` uses `g_keyhan_osal_ops` (ESP-IDF default) |
| `events` | struct | Optional callbacks (any may be `NULL`) |

---

## Events and callbacks

| Callback | When it runs |
|----------|----------------|
| `on_state_changed(from, to, user)` | Agent transitions state (checking, downloading, applying, error, …) |
| `on_progress(downloaded, total, user)` | During image download |
| `on_error(err, user)` | On failure; state becomes `KEYHAN_AGENT_STATE_ERROR` |
| `on_update_ready(user)` | Download finished and `auto_apply == false` |

### Agent states

| State | Meaning |
|-------|---------|
| `KEYHAN_AGENT_STATE_IDLE` | Initialized, not started |
| `KEYHAN_AGENT_STATE_STARTING` | Entered via `keyhan_agent_start()` |
| `KEYHAN_AGENT_STATE_CHECKING` | Fetching manifest |
| `KEYHAN_AGENT_STATE_DOWNLOADING` | Pulling firmware |
| `KEYHAN_AGENT_STATE_APPLYING` | Committing update |
| `KEYHAN_AGENT_STATE_UPDATED` | Update staged or applied |
| `KEYHAN_AGENT_STATE_RUNNING` | No newer firmware |
| `KEYHAN_AGENT_STATE_STOPPED` | After `keyhan_agent_stop()` |
| `KEYHAN_AGENT_STATE_ERROR` | Failure; see `on_error` |

Log errors with the compile-time helper:

```c
ESP_LOGE(TAG, "%s", KEYHAN_AGENT_ERROR_TO_NAME(err));
```

---

## Operational modes

### Automatic apply (default for examples)

```c
.auto_apply = true,
.auto_reboot = false,
```

The agent downloads and calls `apply_staged_update()` in one `step()`. Set `auto_reboot = true` if the platform should reset immediately.

### Manual apply (user confirmation)

```c
.auto_apply = false,
```

After download, `on_update_ready` fires. Your UI or logic can prompt the user, then call OSAL apply yourself (custom code or a future agent API).

### Periodic polling

```c
while (true) {
    keyhan_agent_step(agent);
    vTaskDelay(pdMS_TO_TICKS(5000));
}
```

Each `step()` performs a full check-and-update cycle. Avoid calling `step()` re-entrantly from multiple tasks without synchronization.

---

## Running the ESP-IDF example

**Path:** `example/esp32-sync-http`

1. Install [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) and export the environment.
2. Edit `main/main.c`:
   - Wi‑Fi SSID and password
   - `manifest_url`
   - `DEVICE_TOKEN`
3. Build and flash:

   ```bash
   cd example/esp32-sync-http
   idf.py set-target esp32
   idf.py build flash monitor
   ```

4. Run an HTTP server on your LAN that serves the binary manifest and firmware file.

The example stores version `0` in NVS on first boot, connects to Wi‑Fi, starts the agent, and polls every 5 seconds.

---

## Troubleshooting

| Symptom | Likely cause | What to do |
|---------|----------------|------------|
| `KEYHAN_AGENT_ERR_NETWORK` | Wi‑Fi down, wrong URL, server unreachable | Verify connectivity; test URL with `curl` from your PC |
| `KEYHAN_AGENT_ERR_INTEGRITY` | Manifest not binary, wrong footer, length mismatch | Compare payload with [protocol spec](developer-guide.md#ota-protocol-v1); check `image_size` vs file size |
| Agent state `ERROR` after manifest | Invalid `msg_type`, version ≠ 1, URL too long | Regenerate manifest; keep URL &lt; 255 bytes |
| Download OK but device unchanged | `auto_apply` false or apply only updates NVS in demo | Implement real OTA partition writes in OSAL |
| Always “running”, never updates | `current_version` ≥ manifest `target_version` | Bump server target or reset NVS version in dev |
| Zephyr example does not build | Example uses older init API | Prefer ESP-IDF example or align with `keyhan_agent_config_t` API |

---

## Glossary

| Term | Definition |
|------|------------|
| **Agent** | Keyhan state machine that orchestrates check → download → apply |
| **Manifest** | Server response describing available firmware |
| **OSAL** | Platform hooks for HTTP, flash, and reboot |
| **Stage** | Write firmware bytes to non-running slot or buffer before swap |
| **Apply** | Mark new image bootable and/or reboot |

For API details, error codes, and porting, see the [Developer Guide](developer-guide.md).
