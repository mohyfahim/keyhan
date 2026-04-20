#pragma once

#include "keyhan/agent.h"
#include "keyhan/error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  KEYHAN_AGENT_TRANSPORT_HTTP,
  KEYHAN_AGENT_TRANSPORT_MQTT,
  KEYHAN_AGENT_TRANSPORT_SOCKET,
} keyhan_agent_transport_type_t;

typedef enum {
  KEYHAN_AGENT_TRANSPORT_MSG_TYPE_HELLO
} keyhan_agent_transport_msg_type_t;

typedef struct {
  int size;
  keyhan_agent_transport_msg_type_t type;
} keyhan_agent_transport_msg_header_t;

typedef struct {
  uint8_t payload[64];
} keyhan_agent_transport_msg_body_t;

typedef struct {
  keyhan_agent_transport_msg_header_t hdr;
  keyhan_agent_transport_msg_body_t body;
} keyhan_agent_transport_msg_t;

keyhan_agent_error_t
keyhan_agent_transport_init(keyhan_agent_t *agent,
                            keyhan_agent_transport_type_t type);

#ifdef __cplusplus
}
#endif
