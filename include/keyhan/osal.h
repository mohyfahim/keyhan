#pragma once
#include "keyhan/agent.h"
#include "keyhan/error.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

keyhan_agent_error_t keyhan_osal_transport_http_init(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_osal_transport_http_post(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_osal_transport_http_get(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_osal_transport_http_deinit(keyhan_agent_t *agent);
#ifdef __cplusplus
}
#endif
