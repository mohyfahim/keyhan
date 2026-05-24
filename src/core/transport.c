#include "keyhan/agent.h"

/*
 * Legacy transport API placeholders.
 *
 * The OTA flow is now driven by OSAL OTA operations exposed through
 * keyhan_agent_init_params_t::osal_ops.
 */
keyhan_agent_error_t keyhan_agent_transport_init(keyhan_agent_t *agent) {
  (void)agent;
  return KEYHAN_AGENT_ERR_NOT_SUPPORTED;
}

keyhan_agent_error_t keyhan_agent_transport_pull(keyhan_agent_t *agent) {
  (void)agent;
  return KEYHAN_AGENT_ERR_NOT_SUPPORTED;
}

keyhan_agent_error_t keyhan_agent_transport_stop(keyhan_agent_t *agent) {
  (void)agent;
  return KEYHAN_AGENT_ERR_NOT_SUPPORTED;
}

keyhan_agent_error_t keyhan_agent_transport_deinit(keyhan_agent_t *agent) {
  (void)agent;
  return KEYHAN_AGENT_ERR_NOT_SUPPORTED;
}
