#pragma once
#include "keyhan/error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct keyhan_utils_fifo keyhan_utils_fifo_t;

keyhan_agent_error_t keyhan_utils_fifo_init(keyhan_utils_fifo_t **fifo_out,
                                            size_t item_size, size_t capacity);
bool keyhan_utils_fifo_is_full(const keyhan_utils_fifo_t *fifo);
bool keyhan_utils_fifo_is_empty(const keyhan_utils_fifo_t *fifo);
keyhan_agent_error_t keyhan_utils_fifo_push(keyhan_utils_fifo_t *fifo,
                                            const void *item);
keyhan_agent_error_t keyhan_utils_fifo_pop(keyhan_utils_fifo_t *fifo,
                                           void *out_item);
void keyhan_utils_fifo_deinit(keyhan_utils_fifo_t *fifo);
#ifdef __cplusplus
}
#endif
