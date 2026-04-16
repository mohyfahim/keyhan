#include "keyhan/osal.h"

#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/time_units.h>

#ifndef OTA_ZEPHYR_MAX_TASKS
#define OTA_ZEPHYR_MAX_TASKS 4
#endif

struct ota_osal_task {
  struct k_thread thread;
  k_tid_t tid;
  k_thread_stack_t *stack;
  size_t stack_size;
  ota_osal_task_entry_t entry;
  void *arg;
  bool finished;
};

struct ota_osal_mutex {
  struct k_mutex mutex;
};

static void zephyr_task_trampoline(void *p1, void *p2, void *p3) {
  struct ota_osal_task *task = (struct ota_osal_task *)p1;
  (void)p2;
  (void)p3;

  if (task && task->entry) {
    task->entry(task->arg);
  }

  task->finished = true;
}

int ota_osal_task_create(ota_osal_task_t **task_out,
                         const ota_osal_task_params_t *params) {
  struct ota_osal_task *task;

  if (!task_out || !params || !params->entry || params->stack_size == 0) {
    return -1;
  }

  task = (struct ota_osal_task *)calloc(1, sizeof(*task));
  if (!task) {
    return -1;
  }

  task->stack = (k_thread_stack_t *)k_malloc(params->stack_size);
  if (!task->stack) {
    free(task);
    return -1;
  }

  task->stack_size = params->stack_size;
  task->entry = params->entry;
  task->arg = params->arg;
  task->finished = false;

  task->tid = k_thread_create(&task->thread, task->stack, task->stack_size,
                              zephyr_task_trampoline, task, NULL, NULL,
                              params->priority, 0, K_NO_WAIT);

  if (!task->tid) {
    k_free(task->stack);
    free(task);
    return -1;
  }

  if (params->name) {
    k_thread_name_set(task->tid, params->name);
  }

  *task_out = task;
  return 0;
}

int ota_osal_task_join(ota_osal_task_t *task, uint32_t timeout_ms) {
  int64_t end = k_uptime_get() + timeout_ms;

  if (!task) {
    return -1;
  }

  while (!task->finished) {
    if (timeout_ms != 0xFFFFFFFFU && k_uptime_get() >= end) {
      return -1;
    }
    k_sleep(K_MSEC(10));
  }

  return 0;
}

int ota_osal_task_destroy(ota_osal_task_t *task) {
  if (!task) {
    return -1;
  }

  if (!task->finished && task->tid) {
    k_thread_abort(task->tid);
  }

  if (task->stack) {
    k_free(task->stack);
  }

  free(task);
  return 0;
}

void ota_osal_sleep_ms(uint32_t ms) { k_sleep(K_MSEC(ms)); }

uint64_t ota_osal_time_ms(void) { return (uint64_t)k_uptime_get(); }

int ota_osal_mutex_create(ota_osal_mutex_t **mtx_out) {
  struct ota_osal_mutex *mtx;

  if (!mtx_out) {
    return -1;
  }

  mtx = (struct ota_osal_mutex *)calloc(1, sizeof(*mtx));
  if (!mtx) {
    return -1;
  }

  if (k_mutex_init(&mtx->mutex) != 0) {
    free(mtx);
    return -1;
  }

  *mtx_out = mtx;
  return 0;
}

int ota_osal_mutex_lock(ota_osal_mutex_t *mtx, uint32_t timeout_ms) {
  k_timeout_t timeout;

  if (!mtx) {
    return -1;
  }

  timeout = (timeout_ms == 0xFFFFFFFFU) ? K_FOREVER : K_MSEC(timeout_ms);
  return k_mutex_lock(&mtx->mutex, timeout);
}

int ota_osal_mutex_unlock(ota_osal_mutex_t *mtx) {
  if (!mtx) {
    return -1;
  }

  return k_mutex_unlock(&mtx->mutex);
}

int ota_osal_mutex_destroy(ota_osal_mutex_t *mtx) {
  if (!mtx) {
    return -1;
  }

  free(mtx);
  return 0;
}