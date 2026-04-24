#include "keyhan/transport.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct keyhan_utils_fifo {
  uint8_t *buffer;  // Pointer to the actual data array
  size_t item_size; // Size of one element (e.g.,
                    // sizeof(keyhan_agent_transport_msg_t))
  size_t capacity;  // Maximum number of items
  size_t head;      // Write index
  size_t tail;      // Read index
  size_t count;     // Current number of items
} keyhan_utils_fifo_t;

// Initialize the FIFO with a pre-allocated buffer
keyhan_agent_error_t keyhan_utils_fifo_init(keyhan_utils_fifo_t **fifo_out,
                                            size_t item_size, size_t capacity) {

  if (!fifo_out)
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  keyhan_utils_fifo_t *fifo;
  fifo = (keyhan_utils_fifo_t *)calloc(1, sizeof(keyhan_utils_fifo_t));
  fifo->buffer = (uint8_t *)calloc(capacity, item_size);
  fifo->item_size = item_size;
  fifo->capacity = capacity;
  fifo->head = 0;
  fifo->tail = 0;
  fifo->count = 0;
  *fifo_out = fifo;
  return KEYHAN_AGENT_OK;
}

void keyhan_utils_fifo_deinit(keyhan_utils_fifo_t *fifo) {
  if (fifo->buffer != NULL)
    free(fifo->buffer);
}

bool keyhan_utils_fifo_is_full(const keyhan_utils_fifo_t *fifo) {
  return fifo->count >= fifo->capacity;
}

bool keyhan_utils_fifo_is_empty(const keyhan_utils_fifo_t *fifo) {
  return fifo->count == 0;
}

// Push a keyhan_utils item (copies memory)
keyhan_agent_error_t keyhan_utils_fifo_push(keyhan_utils_fifo_t *fifo,
                                            const void *item) {
  if (keyhan_utils_fifo_is_full(fifo))
    return KEYHAN_AGENT_ERR_BUFFER_FULL;

  size_t offset = fifo->head * fifo->item_size;
  memcpy(fifo->buffer + offset, item, fifo->item_size);

  fifo->head = (fifo->head + 1) % fifo->capacity;
  fifo->count++;
  return KEYHAN_AGENT_OK;
}

// Pop a keyhan_utils item (copies memory)
keyhan_agent_error_t keyhan_utils_fifo_pop(keyhan_utils_fifo_t *fifo,
                                           void *out_item) {
  if (keyhan_utils_fifo_is_empty(fifo))
    return KEYHAN_AGENT_ERR_BUFFER_EMPTY;

  size_t offset = fifo->tail * fifo->item_size;
  memcpy(out_item, fifo->buffer + offset, fifo->item_size);

  fifo->tail = (fifo->tail + 1) % fifo->capacity;
  fifo->count--;
  return KEYHAN_AGENT_OK;
}