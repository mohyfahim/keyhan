#include "keyhan/error.h"

const char *keyhan_agent_error_to_name(keyhan_agent_error_t err) {
  switch (err) {
  case KEYHAN_AGENT_OK:
    return "KEYHAN_AGENT_OK";
  case KEYHAN_AGENT_ERR_INVALID_ARG:
    return "KEYHAN_AGENT_ERR_INVALID_ARG";
  case KEYHAN_AGENT_ERR_NO_MEM:
    return "KEYHAN_AGENT_ERR_NO_MEM";
  case KEYHAN_AGENT_ERR_OS:
    return "KEYHAN_AGENT_ERR_OS";
  case KEYHAN_AGENT_ERR_STATE:
    return "KEYHAN_AGENT_ERR_STATE";
  case KEYHAN_AGENT_ERR_UNINITIALIZED:
    return "KEYHAN_AGENT_ERR_UNINITIALIZED";
  default:
    return "unsupported error";
  }
}