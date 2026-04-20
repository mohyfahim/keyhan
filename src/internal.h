#pragma once
#include "keyhan/agent.h"
#include "keyhan/osal.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct keyhan_agent {
  keyhan_agent_state_t state;
};

#ifdef __cplusplus
}
#endif