#pragma once
#include "keyhan/error.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int version;
  uint32_t image_size;
  char image_url[256];
} keyhan_ota_update_info_t;

typedef void (*keyhan_ota_progress_cb_t)(uint32_t downloaded, uint32_t total,
                                         void *user);

typedef struct {
  keyhan_agent_error_t (*fetch_update_info)(void *ctx, const char *device_token,
                                            keyhan_ota_update_info_t *out_info);
  keyhan_agent_error_t (*download_and_stage)(void *ctx,
                                             const keyhan_ota_update_info_t *info,
                                             keyhan_ota_progress_cb_t progress_cb,
                                             void *progress_user);
  keyhan_agent_error_t (*apply_staged_update)(void *ctx, int target_version,
                                              int auto_reboot);
} keyhan_osal_ota_ops_t;

#ifdef __cplusplus
}
#endif
