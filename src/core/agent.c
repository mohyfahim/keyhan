#include "keyhan/agent.h"
#include "internal.h"
#include "keyhan/error.h"
#include "keyhan/transport.h"

#include <stdlib.h>

keyhan_agent_error_t
keyhan_agent_init(keyhan_agent_t **agent_out,
                  const keyhan_agent_device_info_t *devinfo,
                  const keyhan_agent_init_params_t *params,
                  const keyhan_agent_callbacks_t *cb) {

  if (!agent_out) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }
  keyhan_agent_t *agent;
  agent = (keyhan_agent_t *)calloc(1, sizeof(keyhan_agent_t));
  agent->state = KEYHAN_AGENT_STATE_IDLE;

  *agent_out = agent;

  return KEYHAN_AGENT_OK;
}

keyhan_agent_error_t keyhan_agent_start(keyhan_agent_t *agent) {

  keyhan_agent_transport_init(agent, KEYHAN_AGENT_TRANSPORT_HTTP);

  agent->state = KEYHAN_AGENT_STATE_RUNNING;

  return KEYHAN_AGENT_OK;
}

keyhan_agent_error_t keyhan_agent_step(keyhan_agent_t *agent) {

  keyhan_agent_transport_pull(agent);

  agent->state = KEYHAN_AGENT_STATE_RUNNING;

  return KEYHAN_AGENT_OK;
}

keyhan_agent_error_t keyhan_agent_stop(keyhan_agent_t *agent) {

  keyhan_agent_transport_stop(agent);

  agent->state = KEYHAN_AGENT_STATE_STOPPED;

  return KEYHAN_AGENT_OK;
}

keyhan_agent_error_t keyhan_agent_deinit(keyhan_agent_t *agent) {

  if (agent) {
    keyhan_agent_transport_deinit(agent);
    free(agent);
  }
  return KEYHAN_AGENT_OK;
}