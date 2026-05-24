#pragma once
#include "keyhan/agent.h"
#include "keyhan/utils.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct keyhan_agent {
  keyhan_agent_state_t state;
  keyhan_agent_device_info_t *devinfo;
  keyhan_agent_init_params_t *params;
  keyhan_agent_callbacks_t *cb;
  keyhan_ota_update_info_t pending_update;
};

#ifdef __cplusplus
}
#endif