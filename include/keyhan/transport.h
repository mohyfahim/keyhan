#pragma once

#include "keyhan/agent.h"
#include "keyhan/error.h"

#define KEYHAN_FOOTER_MAGIC 0xDEADBEEF // 4-byte integrity pattern
#define KEYHAN_AGENT_MAX_MSG_SIZE sizeof(keyhan_agent_transport_msg_t)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct keyhan_osal_transport_client keyhan_agent_transport_client_t;

typedef enum {
  KEYHAN_AGENT_TRANSPORT_MSG_TYPE_REQ_HELLO,
  KEYHAN_AGENT_TRANSPORT_MSG_TYPE_RES_HELLO
} keyhan_agent_transport_msg_type_t;

typedef struct __attribute__((packed)) {
  uint8_t version;
  uint32_t payload_len;
  uint8_t type;
} keyhan_agent_transport_msg_header_t;

typedef struct __attribute__((packed)) {
  uint32_t agent_id;
  uint16_t capabilities;
} keyhan_agent_transport_msg_body_req_hello_t;

typedef struct __attribute__((packed)) {
  uint32_t agent_id;
  uint16_t capabilities;
} keyhan_agent_transport_msg_body_res_hello_t;

typedef union {
  keyhan_agent_transport_msg_body_req_hello_t req_hello;
  keyhan_agent_transport_msg_body_res_hello_t res_hello;
  uint8_t raw[64];
} keyhan_agent_transport_msg_body_t;

typedef struct __attribute__((packed)) {
  keyhan_agent_transport_msg_header_t hdr;
  keyhan_agent_transport_msg_body_t body;
  uint32_t footer;
} keyhan_agent_transport_msg_t;

keyhan_agent_error_t keyhan_agent_transport_init(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_agent_transport_pull(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_agent_transport_stop(keyhan_agent_t *agent);
keyhan_agent_error_t keyhan_agent_transport_deinit(keyhan_agent_t *agent);
#ifdef __cplusplus
}
#endif
