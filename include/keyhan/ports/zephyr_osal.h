// #pragma once

// #include "keyhan/osal.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

// typedef keyhan_agent_error_t (*keyhan_zephyr_http_get_manifest_t)(
//     void *user, const char *url, const char *device_token, char *out_buf,
//     size_t out_buf_len, size_t *out_len);

// typedef keyhan_agent_error_t (*keyhan_zephyr_download_image_t)(
//     void *user, const char *url,
//     keyhan_agent_error_t (*on_chunk)(void *chunk_user, const uint8_t *data,
//                                      size_t len),
//     void *chunk_user);

// typedef struct {
//   const char *manifest_url;
//   keyhan_zephyr_http_get_manifest_t http_get_manifest;
//   keyhan_zephyr_download_image_t download_image;
//   keyhan_agent_error_t (*stage_chunk)(void *user, const uint8_t *data,
//                                       size_t len);
//   keyhan_agent_error_t (*apply_staged)(void *user, int target_version,
//                                        int auto_reboot);
//   void *user;
// } keyhan_zephyr_osal_ctx_t;

// extern const keyhan_osal_ota_ops_t g_keyhan_zephyr_osal_ops;

// #ifdef __cplusplus
// }
// #endif
