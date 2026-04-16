#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct keyhan_osal_task keyhan_osal_task_t;
typedef void (*keyhan_osal_task_entry_t)(void *arg);

typedef struct {
  const char *name;
  keyhan_osal_task_entry_t entry;
  void *arg;
  size_t stack_size;
  int priority;
} keyhan_osal_task_params_t;

/* Task API */
int keyhan_osal_task_create(keyhan_osal_task_t **task,
                            const keyhan_osal_task_params_t *params);
int keyhan_osal_task_join(keyhan_osal_task_t *task, uint32_t timeout_ms);
int keyhan_osal_task_destroy(keyhan_osal_task_t *task);

/* Sleep / time */
void keyhan_osal_sleep_ms(uint32_t ms);
uint64_t keyhan_osal_time_ms(void);

/* Mutex API */
typedef struct keyhan_osal_mutex keyhan_osal_mutex_t;

int keyhan_osal_mutex_create(keyhan_osal_mutex_t **mtx);
int keyhan_osal_mutex_lock(keyhan_osal_mutex_t *mtx, uint32_t timeout_ms);
int keyhan_osal_mutex_unlock(keyhan_osal_mutex_t *mtx);
int keyhan_osal_mutex_destroy(keyhan_osal_mutex_t *mtx);

#ifdef __cplusplus
}
#endif
