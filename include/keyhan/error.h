#pragma once

#define KEYHAN_AGENT_ERROR_LIST                                                \
  X(KEYHAN_AGENT_OK, 0)                                                        \
  X(KEYHAN_AGENT_ERR_INVALID_ARG, -1)                                          \
  X(KEYHAN_AGENT_ERR_NO_MEM, -2)                                               \
  X(KEYHAN_AGENT_ERR_OS, -3)                                                   \
  X(KEYHAN_AGENT_ERR_STATE, -4)                                                \
  X(KEYHAN_AGENT_ERR_UNINITIALIZED, -5)                                        \
  X(KEYHAN_AGENT_ERR_BUFFER_FULL, -6)                                          \
  X(KEYHAN_AGENT_ERR_BUFFER_EMPTY, -7)                                         \
  X(KEYHAN_AGENT_ERR_NO_MEMORY, -8)                                            \
  X(KEYHAN_AGENT_ERR_INVALIDE_RES_VERSION, -9)                                 \
  X(KEYHAN_AGENT_ERR_INVALIDE_RES_TYPE, -10)                                   \
  X(KEYHAN_AGENT_ERR_INVALIDE_RES_LEN, -11)                                    \
  X(KEYHAN_AGENT_ERR_INVALIDE_RES_FOOTER, -12)

// Generate enum definition
typedef enum {
#define X(name, value) name = value,
  KEYHAN_AGENT_ERROR_LIST
#undef X
} keyhan_agent_error_t;

// Compile-time string conversion macro
#define KEYHAN_AGENT_ERROR_TO_NAME(err)                                        \
  _Generic((err),                                                              \
      keyhan_agent_error_t: (                                                  \
               (err) == KEYHAN_AGENT_OK ? "KEYHAN_AGENT_OK"                    \
               : (err) == KEYHAN_AGENT_ERR_INVALID_ARG                         \
                   ? "KEYHAN_AGENT_ERR_INVALID_ARG"                            \
               : (err) == KEYHAN_AGENT_ERR_NO_MEM ? "KEYHAN_AGENT_ERR_NO_MEM"  \
               : (err) == KEYHAN_AGENT_ERR_OS     ? "KEYHAN_AGENT_ERR_OS"      \
               : (err) == KEYHAN_AGENT_ERR_STATE  ? "KEYHAN_AGENT_ERR_STATE"   \
               : (err) == KEYHAN_AGENT_ERR_UNINITIALIZED                       \
                   ? "KEYHAN_AGENT_ERR_UNINITIALIZED"                          \
               : (err) == KEYHAN_AGENT_ERR_BUFFER_FULL                         \
                   ? "KEYHAN_AGENT_ERR_BUFFER_FULL"                            \
               : (err) == KEYHAN_AGENT_ERR_BUFFER_EMPTY                        \
                   ? "KEYHAN_AGENT_ERR_BUFFER_EMPTY"                           \
               : (err) == KEYHAN_AGENT_ERR_NO_MEMORY                           \
                   ? "KEYHAN_AGENT_ERR_NO_MEMORY"                              \
               : (err) == KEYHAN_AGENT_ERR_INVALIDE_RES_VERSION                \
                   ? "KEYHAN_AGENT_ERR_INVALIDE_RES_VERSION"                   \
               : (err) == KEYHAN_AGENT_ERR_INVALIDE_RES_TYPE                   \
                   ? "KEYHAN_AGENT_ERR_INVALIDE_RES_TYPE"                      \
               : (err) == KEYHAN_AGENT_ERR_INVALIDE_RES_LEN                    \
                   ? "KEYHAN_AGENT_ERR_INVALIDE_RES_LEN"                       \
               : (err) == KEYHAN_AGENT_ERR_INVALIDE_RES_FOOTER                 \
                   ? "KEYHAN_AGENT_ERR_INVALIDE_RES_FOOTER"                    \
                   : "unsupported error"))

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
