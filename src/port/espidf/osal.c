#include "keyhan/osal.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "keyhan/protocol.h"
#include <string.h>

static const char *TAG = "keyhan_osal_espidf";

typedef struct {
  const char *manifest_url;

} keyhan_espidf_osal_ctx_t;

static keyhan_agent_error_t
keyhan_osal_fetch_update_info(const char *manifest_url,
                              const char *device_token,
                              keyhan_agent_update_t *out_info) {
  esp_http_client_config_t config;
  esp_http_client_handle_t client;
  char response[384] = {0};
  int read_len;
  keyhan_agent_error_t err;

  if (!device_token || !out_info) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  memset(&config, 0, sizeof(config));
  config.url = manifest_url;
  config.method = HTTP_METHOD_GET;
  client = esp_http_client_init(&config);
  if (!client) {
    return KEYHAN_AGENT_ERR_NO_MEM;
  }

  esp_http_client_set_header(client, KEYHAN_OTA_PROTOCOL_HEADER_DEVICE_TOKEN,
                             device_token);
  esp_http_client_set_header(client, "Accept",
                             KEYHAN_OTA_PROTOCOL_HEADER_ACCEPT);
  if (esp_http_client_open(client, 0) != ESP_OK) {
    esp_http_client_cleanup(client);
    return KEYHAN_AGENT_ERR_NETWORK;
  }

  read_len =
      esp_http_client_read_response(client, response, sizeof(response) - 1);
  esp_http_client_close(client);
  esp_http_client_cleanup(client);
  if (read_len <= 0) {
    return KEYHAN_AGENT_ERR_NETWORK;
  }

  err = keyhan_protocol_parse_manifest_v1((const uint8_t *)response,
                                          (size_t)read_len, out_info);
  if (err != KEYHAN_AGENT_OK) {
    ESP_LOGE(TAG, "invalid manifest payload");
    return err;
  }

  return KEYHAN_AGENT_OK;
}

static keyhan_agent_error_t
keyhan_osal_download_and_stage(const keyhan_agent_update_t *info,
                               keyhan_ota_progress_cb_t progress_cb,
                               void *progress_user) {
  esp_http_client_config_t config;
  esp_http_client_handle_t client;
  uint8_t chunk[1024];
  int read_len;
  uint32_t downloaded = 0;

  if (!info || info->image_url[0] == '\0') {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  memset(&config, 0, sizeof(config));
  config.url = info->image_url;
  config.method = HTTP_METHOD_GET;
  client = esp_http_client_init(&config);
  if (!client) {
    return KEYHAN_AGENT_ERR_NO_MEM;
  }

  if (esp_http_client_open(client, 0) != ESP_OK) {
    esp_http_client_cleanup(client);
    return KEYHAN_AGENT_ERR_NETWORK;
  }

  while ((read_len =
              esp_http_client_read(client, (char *)chunk, sizeof(chunk))) > 0) {
    downloaded += (uint32_t)read_len;
    if (progress_cb) {
      progress_cb(downloaded, info->image_size, progress_user);
    }
  }

  esp_http_client_close(client);
  esp_http_client_cleanup(client);
  if (read_len < 0) {
    return KEYHAN_AGENT_ERR_NETWORK;
  }
  if (info->image_size != 0 && downloaded != info->image_size) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }

  return KEYHAN_AGENT_OK;
}

static keyhan_agent_error_t keyhan_osal_apply_staged_update(int target_version,
                                                            int auto_reboot) {
  if (auto_reboot) {
    esp_restart();
  }
  return KEYHAN_AGENT_OK;
}

const keyhan_osal_ota_ops_t g_keyhan_osal_ops = {
    .fetch_update_info = keyhan_osal_fetch_update_info,
    .download_and_stage = keyhan_osal_download_and_stage,
    .apply_staged_update = keyhan_osal_apply_staged_update,
};