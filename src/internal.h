#pragma once
#include "keyhan/agent.h"
#include "keyhan/osal.h"
#include "keyhan/transport.h"
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
  keyhan_agent_transport_client_t *client; // depens on osal
  keyhan_utils_fifo_t *buffer;
};

#ifdef __cplusplus
}
#endif