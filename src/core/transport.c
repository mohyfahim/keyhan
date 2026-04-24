
#include "keyhan/transport.h"
#include "internal.h"
#include "keyhan/osal.h"

#define KEYHAN_AGENT_TRANSPORT_ENDPOINT "http://"

keyhan_agent_error_t keyhan_agent_transport_init(keyhan_agent_t *agent) {

  keyhan_agent_error_t err = keyhan_utils_fifo_init(
      &agent->buffer, sizeof(keyhan_agent_transport_msg_t), 4);
  if (err != KEYHAN_AGENT_OK) {
    return err;
  }
#ifdef CONFIG_KEYHAN_TRANSPORT_USE_HTTP
  err = keyhan_osal_transport_http_init(agent);

  if (err != KEYHAN_AGENT_OK) {
    return err;
  }
  err = keyhan_osal_transport_http_post(agent);

  if (err != KEYHAN_AGENT_OK) {
    return err;
  }
#elif defined(CONFIG_KEYHAN_TRANSPORT_USE_SOCKET)

#endif

  return KEYHAN_AGENT_OK;
}
keyhan_agent_error_t keyhan_agent_transport_pull(keyhan_agent_t *agent) {

#ifdef CONFIG_KEYHAN_TRANSPORT_USE_HTTP
  keyhan_osal_transport_http_post(agent);
#elif defined(CONFIG_KEYHAN_TRANSPORT_USE_SOCKET)

#endif
  return KEYHAN_AGENT_OK;
}
keyhan_agent_error_t keyhan_agent_transport_stop(keyhan_agent_t *agent) {

#ifdef CONFIG_KEYHAN_TRANSPORT_USE_HTTP
  // TODO: make a log library with osal
#elif defined(CONFIG_KEYHAN_TRANSPORT_USE_SOCKET)

#endif
  return KEYHAN_AGENT_OK;
}
keyhan_agent_error_t keyhan_agent_transport_deinit(keyhan_agent_t *agent) {

#ifdef CONFIG_KEYHAN_TRANSPORT_USE_HTTP
  keyhan_osal_transport_http_deinit(agent);
#elif defined(CONFIG_KEYHAN_TRANSPORT_USE_SOCKET)

#endif
  return KEYHAN_AGENT_OK;
}
