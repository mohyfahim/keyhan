#pragma once

#include "keyhan/agent.h"
#include "keyhan/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Legacy declarations kept for source compatibility. */
keyhan_agent_error_t keyhan_agent_transport_init(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_agent_transport_pull(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_agent_transport_stop(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_agent_transport_deinit(keyhan_agent_t *agent);
#ifdef __cplusplus
}
#endif
