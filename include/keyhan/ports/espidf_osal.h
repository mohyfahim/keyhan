#pragma once

#include "keyhan/osal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char *manifest_url;
  keyhan_agent_error_t (*stage_chunk)(void *user, const uint8_t *data,
                                      size_t len);
  keyhan_agent_error_t (*apply_staged)(void *user, int target_version,
                                       int auto_reboot);
  void *user;
} keyhan_espidf_osal_ctx_t;

extern const keyhan_osal_ota_ops_t g_keyhan_espidf_osal_ops;

#ifdef __cplusplus
}
#endif
