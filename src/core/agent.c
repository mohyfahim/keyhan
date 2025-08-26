#include "keyhan/agent.h"
#include "keyhan/error.h"

struct keyhan_agent {
  int state;
  void *platform;
  int retry_count;
};

keyhan_agent_error_t keyhan_agent_init(keyhan_agent_t *agent,
                                       keyhan_agent_device_info_t *devinfo,
                                       const keyhan_agent_init_params_t *params,
                                       keyhan_agent_callbacks_t *cb) {

  return KEYHAN_AGENT_OK;
}