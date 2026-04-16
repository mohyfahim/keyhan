#pragma once

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
  KEYHAN_AGENT_OK,
  KEYHAN_AGENT_ERR_INVALID_ARG = -1,
  KEYHAN_AGENT_ERR_NO_MEM = -2,
  KEYHAN_AGENT_ERR_OS = -3,
  KEYHAN_AGENT_ERR_STATE = -4,
} keyhan_agent_error_t;
#ifdef __cplusplus
}
#endif