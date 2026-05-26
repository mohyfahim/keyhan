#pragma once
#include "keyhan/agent.h"
#include "keyhan/osal.h"
#include "keyhan/utils.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// struct keyhan_agent {
//   keyhan_agent_state_t state;
//   keyhan_agent_device_info_t *devinfo;
//   keyhan_agent_init_params_t *params;
//   keyhan_agent_callbacks_t *cb;
//   keyhan_ota_update_info_t pending_update;
// };

struct keyhan_agent {
  keyhan_agent_state_t state;
  keyhan_agent_config_t cfg; /* copied */
  keyhan_agent_update_t pending;
  const keyhan_osal_ota_ops_t *osal_ops;
  void *osal_ctx;
};

#ifdef __cplusplus
}
#endif