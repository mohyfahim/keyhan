
#include "keyhan/transport.h"
#include "internal.h"
#include "keyhan/osal.h"
#include <string.h>

#define KEYHAN_AGENT_TRANSPORT_ENDPOINT "http://"
keyhan_agent_error_t keyhan_agent_transport_init(keyhan_agent_t *agent) {

  keyhan_agent_error_t err;
  keyhan_agent_transport_msg_t msg = {};
  msg.hdr.payload_len = sizeof(keyhan_agent_transport_msg_header_t) +
                        sizeof(keyhan_agent_transport_msg_body_req_hello_t) +
                        sizeof(uint32_t);
  msg.hdr.version = 1;
  msg.hdr.type = KEYHAN_AGENT_TRANSPORT_MSG_TYPE_REQ_HELLO;

  msg.body.req_hello.agent_id = 1;
  msg.body.req_hello.capabilities = 1;

  msg.footer = KEYHAN_FOOTER_MAGIC;

  uint8_t serialize[KEYHAN_AGENT_MAX_MSG_SIZE] = {};
  uint8_t deserialize[KEYHAN_AGENT_MAX_MSG_SIZE] = {};

  size_t len = 0;
  memcpy(serialize + len, &msg.hdr,
         sizeof(keyhan_agent_transport_msg_header_t));
  len += sizeof(keyhan_agent_transport_msg_header_t);
  memcpy(serialize + len, &msg.body,
         sizeof(keyhan_agent_transport_msg_body_req_hello_t));
  len += sizeof(keyhan_agent_transport_msg_body_req_hello_t);
  memcpy(serialize + len, &msg.footer, sizeof(msg.footer));
  len += sizeof(msg.footer);

#ifdef CONFIG_KEYHAN_TRANSPORT_USE_HTTP
  err = keyhan_osal_transport_http_init(agent);

  if (err != KEYHAN_AGENT_OK) {
    return err;
  }
  err = keyhan_osal_transport_http_post(
      agent, "http://192.168.1.108:8091/v1/device", serialize, len);

  if (err != KEYHAN_AGENT_OK) {
    return err;
  }

#elif defined(CONFIG_KEYHAN_TRANSPORT_USE_SOCKET)

#endif

  // parse agent response
  err = keyhan_utils_fifo_pop(agent->buffer, deserialize);
  if (err != KEYHAN_AGENT_OK) {
    return err;
  }

  len = 0;
  memcpy(&msg.hdr, deserialize, sizeof(keyhan_agent_transport_msg_header_t));
  len += sizeof(keyhan_agent_transport_msg_header_t);
  if (msg.hdr.version != 1) {
    return KEYHAN_AGENT_ERR_INVALIDE_RES_VERSION;
  }

  if (msg.hdr.type != KEYHAN_AGENT_TRANSPORT_MSG_TYPE_RES_HELLO) {
    return KEYHAN_AGENT_ERR_INVALIDE_RES_TYPE;
  }

  memcpy(&msg.body, deserialize + len,
         sizeof(keyhan_agent_transport_msg_body_res_hello_t));
  len += sizeof(keyhan_agent_transport_msg_body_res_hello_t);

  memcpy(&msg.footer, deserialize + len, sizeof(msg.footer));
  len += sizeof(msg.footer);

  if (len != msg.hdr.payload_len) {
    return KEYHAN_AGENT_ERR_INVALIDE_RES_LEN;
  }

  if (msg.footer != KEYHAN_FOOTER_MAGIC) {
    return KEYHAN_AGENT_ERR_INVALIDE_RES_FOOTER;
  }
  return KEYHAN_AGENT_OK;
}

keyhan_agent_error_t keyhan_agent_transport_pull(keyhan_agent_t *agent) {

#ifdef CONFIG_KEYHAN_TRANSPORT_USE_HTTP
  // keyhan_osal_transport_http_post(agent);
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
