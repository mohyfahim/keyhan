#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keyhan/agent.h"
#include "keyhan/error.h"
#include "keyhan/ports/zephyr_osal.h"
#include "keyhan/protocol.h"

#define OTA_HOST "192.168.1.50"
#define OTA_PORT 8080
#define OTA_MANIFEST_PATH "/v1/manifest"
#define OTA_DEVICE_TOKEN "esp32-zephyr-device-01"
#define OTA_LOCAL_VERSION 1

typedef struct {
  int current_version;
  uint32_t staged_size;
} app_ctx_t;

static int connect_to_server(const char *host, uint16_t port) {
  struct sockaddr_in addr;
  int sock = -1;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0) {
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (zsock_inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
    close(sock);
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(sock);
    return -1;
  }

  return sock;
}

static keyhan_agent_error_t read_http_body(int sock, char *out_buf,
                                           size_t out_buf_len,
                                           size_t *out_len) {
  char rx[768];
  ssize_t n;
  size_t total = 0;
  char *body;
  size_t body_len;

  if (!out_buf || !out_len || out_buf_len == 0) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  while ((n = recv(sock, rx + total, sizeof(rx) - total - 1, 0)) > 0) {
    total += (size_t)n;
    if (total >= sizeof(rx) - 1) {
      break;
    }
  }

  if (n < 0) {
    return KEYHAN_AGENT_ERR_NETWORK;
  }
  rx[total] = '\0';

  body = strstr(rx, "\r\n\r\n");
  if (!body) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }
  body += 4;
  body_len = total - (size_t)(body - rx);
  if (body_len >= out_buf_len) {
    return KEYHAN_AGENT_ERR_NO_MEM;
  }

  memcpy(out_buf, body, body_len);
  out_buf[body_len] = '\0';
  *out_len = body_len;
  return KEYHAN_AGENT_OK;
}

static keyhan_agent_error_t app_http_get_manifest(void *user, const char *url,
                                                  const char *device_token,
                                                  char *out_buf,
                                                  size_t out_buf_len,
                                                  size_t *out_len) {
  int sock;
  char req[384];
  int written;
  (void)user;
  (void)url;

  sock = connect_to_server(OTA_HOST, OTA_PORT);
  if (sock < 0) {
    return KEYHAN_AGENT_ERR_NETWORK;
  }

  written = snprintk(
      req, sizeof(req),
      "GET " OTA_MANIFEST_PATH " HTTP/1.1\r\n"
      "Host: " OTA_HOST ":" STRINGIFY(OTA_PORT) "\r\n"
      "Connection: close\r\n"
      KEYHAN_OTA_PROTOCOL_HEADER_DEVICE_TOKEN ": %s\r\n"
      "Accept: " KEYHAN_OTA_PROTOCOL_HEADER_ACCEPT "\r\n\r\n",
      device_token);
  if (written <= 0 || written >= (int)sizeof(req)) {
    close(sock);
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  if (send(sock, req, (size_t)written, 0) < 0) {
    close(sock);
    return KEYHAN_AGENT_ERR_NETWORK;
  }

  keyhan_agent_error_t err = read_http_body(sock, out_buf, out_buf_len, out_len);
  close(sock);
  return err;
}

static keyhan_agent_error_t app_download_image(
    void *user, const char *url,
    keyhan_agent_error_t (*on_chunk)(void *chunk_user, const uint8_t *data,
                                     size_t len),
    void *chunk_user) {
  int sock;
  char req[384];
  char rx[1024];
  ssize_t n;
  int header_parsed = 0;
  char *body;
  (void)user;
  (void)url;

  sock = connect_to_server(OTA_HOST, OTA_PORT);
  if (sock < 0) {
    return KEYHAN_AGENT_ERR_NETWORK;
  }

  int written = snprintk(req, sizeof(req),
                         "GET /firmware.bin HTTP/1.1\r\n"
                         "Host: " OTA_HOST ":" STRINGIFY(OTA_PORT) "\r\n"
                         "Connection: close\r\n\r\n");
  if (written <= 0 || written >= (int)sizeof(req)) {
    close(sock);
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  if (send(sock, req, (size_t)written, 0) < 0) {
    close(sock);
    return KEYHAN_AGENT_ERR_NETWORK;
  }

  while ((n = recv(sock, rx, sizeof(rx), 0)) > 0) {
    if (!header_parsed) {
      body = strstr(rx, "\r\n\r\n");
      if (!body) {
        continue;
      }
      body += 4;
      size_t body_len = (size_t)n - (size_t)(body - rx);
      header_parsed = 1;
      if (body_len > 0) {
        keyhan_agent_error_t err =
            on_chunk(chunk_user, (const uint8_t *)body, body_len);
        if (err != KEYHAN_AGENT_OK) {
          close(sock);
          return err;
        }
      }
      continue;
    }

    keyhan_agent_error_t err = on_chunk(chunk_user, (const uint8_t *)rx, (size_t)n);
    if (err != KEYHAN_AGENT_OK) {
      close(sock);
      return err;
    }
  }

  close(sock);
  return (n < 0) ? KEYHAN_AGENT_ERR_NETWORK : KEYHAN_AGENT_OK;
}

static keyhan_agent_error_t app_stage_chunk(void *user, const uint8_t *data,
                                            size_t len) {
  app_ctx_t *ctx = (app_ctx_t *)user;
  (void)data;
  if (!ctx) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }
  ctx->staged_size += (uint32_t)len;
  return KEYHAN_AGENT_OK;
}

static keyhan_agent_error_t app_apply_staged(void *user, int target_version,
                                             int auto_reboot) {
  app_ctx_t *ctx = (app_ctx_t *)user;
  if (!ctx) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }
  ctx->current_version = target_version;
  printk("Applied OTA target_version=%d staged_size=%u\n", target_version,
         ctx->staged_size);
  if (auto_reboot) {
    sys_reboot(SYS_REBOOT_COLD);
  }
  return KEYHAN_AGENT_OK;
}

static void on_state_changed(int old_state, int new_state, void *user) {
  (void)user;
  printk("state %d -> %d\n", old_state, new_state);
}

static void on_progress(uint32_t downloaded, uint32_t total, void *user) {
  (void)user;
  printk("progress: %u/%u\n", downloaded, total);
}

static void on_error(keyhan_agent_error_t err, void *user) {
  (void)user;
  printk("error: %s\n", KEYHAN_AGENT_ERROR_TO_NAME(err));
}

static void on_update_ready(void *user) {
  (void)user;
  printk("update ready\n");
}

int main(void) {
  app_ctx_t app = {.current_version = OTA_LOCAL_VERSION, .staged_size = 0};
  keyhan_agent_t *agent = NULL;

  keyhan_zephyr_osal_ctx_t osal_ctx = {
      .manifest_url = "http://" OTA_HOST ":" STRINGIFY(OTA_PORT) OTA_MANIFEST_PATH,
      .http_get_manifest = app_http_get_manifest,
      .download_image = app_download_image,
      .stage_chunk = app_stage_chunk,
      .apply_staged = app_apply_staged,
      .user = &app,
  };

  keyhan_agent_device_info_t devinfo = {
      .device_token = OTA_DEVICE_TOKEN,
      .current_version = OTA_LOCAL_VERSION,
  };

  keyhan_agent_init_params_t params = {
      .auto_apply = true,
      .auto_reboot = false,
      .osal_ops = &g_keyhan_zephyr_osal_ops,
      .osal_ctx = &osal_ctx,
  };

  keyhan_agent_callbacks_t cb = {
      .on_state_changed = on_state_changed,
      .on_progress = on_progress,
      .on_error = on_error,
      .on_update_ready = on_update_ready,
      .user = NULL,
  };

  keyhan_agent_error_t err = keyhan_agent_init(&agent, &devinfo, &params, &cb);
  if (err != KEYHAN_AGENT_OK) {
    printk("init failed: %s\n", KEYHAN_AGENT_ERROR_TO_NAME(err));
    return -1;
  }

  while (1) {
    err = keyhan_agent_step(agent);
    if (err != KEYHAN_AGENT_OK && err != KEYHAN_AGENT_ERR_NO_UPDATE) {
      printk("step failed: %s\n", KEYHAN_AGENT_ERROR_TO_NAME(err));
    }
    k_sleep(K_SECONDS(5));
  }

  return 0;
}
