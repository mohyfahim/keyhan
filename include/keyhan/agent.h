#pragma once

#include "keyhan/error.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct keyhan_agent keyhan_agent_t;

typedef struct {
  const char *project_id;
  const char *device_id;
  const int current_version;
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
} keyhan_agent_init_params_t;

typedef struct {
  const char *name;
  uint32_t task_stack_size;
  int task_priority;
  uint32_t loop_period_ms;
} keyhan_agent_async_cfg_t;

typedef enum {
  OTA_AGENT_STATE_STOPPED = 0,
  OTA_AGENT_STATE_IDLE,
  OTA_AGENT_STATE_RUNNING,
  OTA_AGENT_STATE_STOPPING,
  OTA_AGENT_STATE_ERROR,
} keyhan_agent_state_t;

keyhan_agent_error_t keyhan_agent_init(keyhan_agent_t *agent,
                                       keyhan_agent_device_info_t *devinfo,
                                       const keyhan_agent_init_params_t *params,
                                       keyhan_agent_callbacks_t *cb);
keyhan_agent_error_t keyhan_agent_start(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_agent_start_async(keyhan_agent_t *agent,
                                              keyhan_agent_async_cfg_t *cfg);
keyhan_agent_error_t keyhan_agent_join(keyhan_agent_t *agent,
                                       uint32_t timeout_ms);
keyhan_agent_error_t keyhan_agent_stop_async(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_agent_stop(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_agent_deinit(keyhan_agent_t *agent);

keyhan_agent_state_t keyhan_agent_get_state(keyhan_agent_t *agent);

#ifdef __cplusplus
}
#endif