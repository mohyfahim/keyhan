#include "keyhan/osal.h"
#include "keyhan/ports/zephyr_osal.h"
#include "keyhan/protocol.h"

typedef struct {
  keyhan_zephyr_osal_ctx_t *ctx;
  keyhan_ota_progress_cb_t progress_cb;
  void *progress_user;
  uint32_t downloaded;
  uint32_t total;
} keyhan_zephyr_download_ctx_t;

static keyhan_agent_error_t keyhan_zephyr_on_chunk(void *chunk_user,
                                                    const uint8_t *data,
                                                    size_t len) {
  keyhan_zephyr_download_ctx_t *dl = (keyhan_zephyr_download_ctx_t *)chunk_user;
  keyhan_agent_error_t err;

  if (!dl || !dl->ctx || !dl->ctx->stage_chunk) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  err = dl->ctx->stage_chunk(dl->ctx->user, data, len);
  if (err != KEYHAN_AGENT_OK) {
    return err;
  }

  dl->downloaded += (uint32_t)len;
  if (dl->progress_cb) {
    dl->progress_cb(dl->downloaded, dl->total, dl->progress_user);
  }
  return KEYHAN_AGENT_OK;
}

static keyhan_agent_error_t keyhan_zephyr_fetch_update_info(
    void *ctx, const char *device_token, keyhan_ota_update_info_t *out_info) {
  keyhan_zephyr_osal_ctx_t *osal_ctx = (keyhan_zephyr_osal_ctx_t *)ctx;
  char response[384];
  size_t response_len = 0;
  keyhan_agent_error_t err;

  if (!osal_ctx || !osal_ctx->manifest_url || !osal_ctx->http_get_manifest ||
      !device_token || !out_info) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  err = osal_ctx->http_get_manifest(osal_ctx->user, osal_ctx->manifest_url,
                                    device_token, response, sizeof(response),
                                    &response_len);
  if (err != KEYHAN_AGENT_OK) {
    return err;
  }
  return keyhan_protocol_parse_manifest_v1((const uint8_t *)response, response_len,
                                           out_info);
}

static keyhan_agent_error_t keyhan_zephyr_download_and_stage(
    void *ctx, const keyhan_ota_update_info_t *info,
    keyhan_ota_progress_cb_t progress_cb, void *progress_user) {
  keyhan_zephyr_osal_ctx_t *osal_ctx = (keyhan_zephyr_osal_ctx_t *)ctx;
  keyhan_zephyr_download_ctx_t download_ctx;
  keyhan_agent_error_t err;

  if (!osal_ctx || !osal_ctx->download_image || !osal_ctx->stage_chunk || !info ||
      info->image_url[0] == '\0') {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  download_ctx.ctx = osal_ctx;
  download_ctx.progress_cb = progress_cb;
  download_ctx.progress_user = progress_user;
  download_ctx.downloaded = 0;
  download_ctx.total = info->image_size;

  err = osal_ctx->download_image(osal_ctx->user, info->image_url,
                                 keyhan_zephyr_on_chunk, &download_ctx);
  if (err != KEYHAN_AGENT_OK) {
    return err;
  }

  if (info->image_size != 0 && download_ctx.downloaded != info->image_size) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }
  return KEYHAN_AGENT_OK;
}

static keyhan_agent_error_t keyhan_zephyr_apply_staged_update(void *ctx,
                                                              int target_version,
                                                              int auto_reboot) {
  keyhan_zephyr_osal_ctx_t *osal_ctx = (keyhan_zephyr_osal_ctx_t *)ctx;
  if (!osal_ctx || !osal_ctx->apply_staged) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }
  return osal_ctx->apply_staged(osal_ctx->user, target_version, auto_reboot);
}

const keyhan_osal_ota_ops_t g_keyhan_zephyr_osal_ops = {
    .fetch_update_info = keyhan_zephyr_fetch_update_info,
    .download_and_stage = keyhan_zephyr_download_and_stage,
    .apply_staged_update = keyhan_zephyr_apply_staged_update,
};