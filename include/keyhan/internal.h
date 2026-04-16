#pragma once
#include "keyhan/agent.h"
#include "keyhan/osal.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct keyhan_agent {
  keyhan_agent_async_cfg_t cfg;

  keyhan_osal_task_t *task;
  keyhan_osal_mutex_t *lock;

  volatile bool stop_requested;
  volatile bool started;

  keyhan_agent_state_t state;
};

#ifdef __cplusplus
}
#endif