#pragma once

typedef enum {
  KEYHAN_AGENT_OK,
  KEYHAN_AGENT_ERR_INVALID_ARG = -1,
  KEYHAN_AGENT_ERR_NO_MEM = -2,
  KEYHAN_AGENT_ERR_OS = -3,
  KEYHAN_AGENT_ERR_STATE = -4,
  KEYHAN_AGENT_ERR_UNINITIALIZED = -5,
  KEYHAN_AGENT_ERR_BUFFER_FULL = -6,
  KEYHAN_AGENT_ERR_BUFFER_EMPTY = -7,

} keyhan_agent_error_t;

#ifdef __cplusplus
extern "C" {
#endif

const char *keyhan_agent_error_to_name(keyhan_agent_error_t err);

#ifdef __cplusplus
}
#endif
