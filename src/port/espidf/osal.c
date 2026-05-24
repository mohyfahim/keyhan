#include "keyhan/osal.h"
#include "keyhan/ports/espidf_osal.h"
#include "keyhan/protocol.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "keyhan_osal_espidf";

static keyhan_agent_error_t keyhan_osal_fetch_update_info(void *ctx,
                                                          const char *device_token,
                                                          keyhan_ota_update_info_t *out_info) {
  keyhan_espidf_osal_ctx_t *osal_ctx = (keyhan_espidf_osal_ctx_t *)ctx;
  esp_http_client_config_t config;
  esp_http_client_handle_t client;
  char response[384] = {0};
  int read_len;
  keyhan_agent_error_t err;

  if (!osal_ctx || !osal_ctx->manifest_url || !device_token || !out_info) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  memset(&config, 0, sizeof(config));
  config.url = osal_ctx->manifest_url;
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

  read_len = esp_http_client_read_response(client, response, sizeof(response) - 1);
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

static keyhan_agent_error_t keyhan_osal_download_and_stage(
    void *ctx, const keyhan_ota_update_info_t *info,
    keyhan_ota_progress_cb_t progress_cb, void *progress_user) {
  keyhan_espidf_osal_ctx_t *osal_ctx = (keyhan_espidf_osal_ctx_t *)ctx;
  esp_http_client_config_t config;
  esp_http_client_handle_t client;
  uint8_t chunk[1024];
  int read_len;
  uint32_t downloaded = 0;

  if (!osal_ctx || !osal_ctx->stage_chunk || !info || info->image_url[0] == '\0') {
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

  while ((read_len = esp_http_client_read(client, (char *)chunk, sizeof(chunk))) > 0) {
    keyhan_agent_error_t err = osal_ctx->stage_chunk(osal_ctx->user, chunk,
                                                     (size_t)read_len);
    if (err != KEYHAN_AGENT_OK) {
      esp_http_client_close(client);
      esp_http_client_cleanup(client);
      return err;
    }
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

static keyhan_agent_error_t keyhan_osal_apply_staged_update(void *ctx,
                                                            int target_version,
                                                            int auto_reboot) {
  keyhan_espidf_osal_ctx_t *osal_ctx = (keyhan_espidf_osal_ctx_t *)ctx;
  if (!osal_ctx || !osal_ctx->apply_staged) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }
  return osal_ctx->apply_staged(osal_ctx->user, target_version, auto_reboot);
}

const keyhan_osal_ota_ops_t g_keyhan_espidf_osal_ops = {
    .fetch_update_info = keyhan_osal_fetch_update_info,
    .download_and_stage = keyhan_osal_download_and_stage,
    .apply_staged_update = keyhan_osal_apply_staged_update,
};