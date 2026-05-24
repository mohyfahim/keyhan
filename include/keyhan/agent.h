#pragma once

#include "keyhan/error.h"
#include "keyhan/osal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct keyhan_agent keyhan_agent_t;

typedef struct {
  const char *device_token;
  int current_version;
} keyhan_agent_device_info_t;

typedef struct {
  void (*on_state_changed)(int old_state, int new_state, void *user);
  void (*on_progress)(uint32_t downloaded, uint32_t total, void *user);
  void (*on_error)(keyhan_agent_error_t err, void *user);
  void (*on_update_ready)(void *user);
  void *user;
} keyhan_agent_callbacks_t;

typedef struct {
  bool auto_apply;
  bool auto_reboot;
  const keyhan_osal_ota_ops_t *osal_ops;
  void *osal_ctx;
} keyhan_agent_init_params_t;

typedef enum {
  KEYHAN_AGENT_STATE_STOPPED = 0,
  KEYHAN_AGENT_STATE_IDLE,
  KEYHAN_AGENT_STATE_CHECKING,
  KEYHAN_AGENT_STATE_DOWNLOADING,
  KEYHAN_AGENT_STATE_APPLYING,
  KEYHAN_AGENT_STATE_UPDATED,
  KEYHAN_AGENT_STATE_RUNNING, /* Compatibility: means no update pending */
  KEYHAN_AGENT_STATE_STOPPING,
  KEYHAN_AGENT_STATE_ERROR,
} keyhan_agent_state_t;

keyhan_agent_error_t keyhan_agent_init(keyhan_agent_t **agent_out,
                                       keyhan_agent_device_info_t *devinfo,
                                       keyhan_agent_init_params_t *params,
                                       keyhan_agent_callbacks_t *cb);
keyhan_agent_error_t keyhan_agent_start(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_agent_step(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_agent_stop(keyhan_agent_t *agent);

keyhan_agent_error_t keyhan_agent_deinit(keyhan_agent_t *agent);

#ifdef __cplusplus
}
#endif