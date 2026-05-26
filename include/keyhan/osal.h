#pragma once
#include "keyhan/error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int version;
  uint32_t image_size;
  char image_url[256];
} keyhan_agent_update_t;

typedef struct {
  keyhan_agent_error_t (*write_chunk)(void *user, const uint8_t *data,
                                      size_t len);
  keyhan_agent_error_t (*commit)(void *user, int version, bool reboot);
  void *user;
} keyhan_staging_t;

typedef void (*keyhan_ota_progress_cb_t)(uint32_t downloaded, uint32_t total,
                                         void *user);

typedef struct {
  keyhan_agent_error_t (*fetch_update_info)(const char *manifest_url,
                                            const char *device_token,
                                            keyhan_agent_update_t *out_info);
  keyhan_agent_error_t (*download_and_stage)(
      const keyhan_agent_update_t *info, keyhan_ota_progress_cb_t progress_cb,
      void *progress_user);
  keyhan_agent_error_t (*apply_staged_update)(int target_version,
                                              int auto_reboot);

} keyhan_osal_ota_ops_t;

extern const keyhan_osal_ota_ops_t g_keyhan_osal_ops;

#ifdef __cplusplus
}
#endif
