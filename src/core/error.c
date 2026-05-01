#include "keyhan/error.h"

// const char *keyhan_agent_error_to_name(keyhan_agent_error_t err) {
//   // TODO: convert to compile time
//   switch (err) {
//   case KEYHAN_AGENT_OK:
//     return "KEYHAN_AGENT_OK";
//   case KEYHAN_AGENT_ERR_INVALID_ARG:
//     return "KEYHAN_AGENT_ERR_INVALID_ARG";
//   case KEYHAN_AGENT_ERR_NO_MEM:
//     return "KEYHAN_AGENT_ERR_NO_MEM";
//   case KEYHAN_AGENT_ERR_OS:
//     return "KEYHAN_AGENT_ERR_OS";
//   case KEYHAN_AGENT_ERR_STATE:
//     return "KEYHAN_AGENT_ERR_STATE";
//   case KEYHAN_AGENT_ERR_UNINITIALIZED:
//     return "KEYHAN_AGENT_ERR_UNINITIALIZED";
//   case KEYHAN_AGENT_ERR_BUFFER_FULL:
//     return "KEYHAN_AGENT_ERR_BUFFER_FULL";
//   case KEYHAN_AGENT_ERR_BUFFER_EMPTY:
//     return "KEYHAN_AGENT_ERR_BUFFER_EMPTY";
//   case KEYHAN_AGENT_ERR_NO_MEMORY:
//     return "KEYHAN_AGENT_ERR_NO_MEMORY";
//   case KEYHAN_AGENT_ERR_INVALIDE_RES_VERSION:
//     return "KEYHAN_AGENT_ERR_INVALIDE_RES_VERSION";
//   case KEYHAN_AGENT_ERR_INVALIDE_RES_TYPE:
//     return "KEYHAN_AGENT_ERR_INVALIDE_RES_TYPE";
//   case KEYHAN_AGENT_ERR_INVALIDE_RES_LEN:
//     return "KEYHAN_AGENT_ERR_INVALIDE_RES_LEN";
//   case KEYHAN_AGENT_ERR_INVALIDE_RES_FOOTER:
//     return "KEYHAN_AGENT_ERR_INVALIDE_RES_FOOTER";
//   default:
//     return "unsupported error";
//   }
// }