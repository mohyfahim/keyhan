// #include "keyhan/osal.h"

// #include <stdlib.h>
// #include <string.h>

// #include "esp_timer.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/semphr.h"
// #include "freertos/task.h"

// struct ota_osal_task {
//   TaskHandle_t handle;
//   ota_osal_task_entry_t entry;
//   void *arg;
//   volatile int finished;
// };

// struct ota_osal_mutex {
//   SemaphoreHandle_t mutex;
// };

// static void espidf_task_trampoline(void *arg) {
//   struct ota_osal_task *task = (struct ota_osal_task *)arg;

//   if (task && task->entry) {
//     task->entry(task->arg);
//   }

//   task->finished = 1;
//   vTaskDelete(NULL);
// }

// int ota_osal_task_create(ota_osal_task_t **task_out,
//                          const ota_osal_task_params_t *params) {
//   BaseType_t rc;
//   struct ota_osal_task *task;

//   if (!task_out || !params || !params->entry || params->stack_size == 0) {
//     return -1;
//   }

//   task = (struct ota_osal_task *)calloc(1, sizeof(*task));
//   if (!task) {
//     return -1;
//   }

//   task->entry = params->entry;
//   task->arg = params->arg;
//   task->finished = 0;

//   rc = xTaskCreate(espidf_task_trampoline,
//                    params->name ? params->name : "ota_agent",
//                    (uint32_t)(params->stack_size / sizeof(StackType_t)),
//                    task, params->priority, &task->handle);

//   if (rc != pdPASS) {
//     free(task);
//     return -1;
//   }

//   *task_out = task;
//   return 0;
// }

// int ota_osal_task_join(ota_osal_task_t *task, uint32_t timeout_ms) {
//   uint64_t start;

//   if (!task) {
//     return -1;
//   }

//   start = ota_osal_time_ms();

//   while (!task->finished) {
//     if (timeout_ms != 0xFFFFFFFFU) {
//       uint64_t now = ota_osal_time_ms();
//       if ((now - start) >= timeout_ms) {
//         return -1;
//       }
//     }
//     vTaskDelay(pdMS_TO_TICKS(10));
//   }

//   return 0;
// }

// int ota_osal_task_destroy(ota_osal_task_t *task) {
//   if (!task) {
//     return -1;
//   }

//   if (!task->finished && task->handle) {
//     vTaskDelete(task->handle);
//   }

//   free(task);
//   return 0;
// }

// void ota_osal_sleep_ms(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }

// uint64_t ota_osal_time_ms(void) {
//   return (uint64_t)(esp_timer_get_time() / 1000ULL);
// }

// int ota_osal_mutex_create(ota_osal_mutex_t **mtx_out) {
//   struct ota_osal_mutex *mtx;

//   if (!mtx_out) {
//     return -1;
//   }

//   mtx = (struct ota_osal_mutex *)calloc(1, sizeof(*mtx));
//   if (!mtx) {
//     return -1;
//   }

//   mtx->mutex = xSemaphoreCreateMutex();
//   if (!mtx->mutex) {
//     free(mtx);
//     return -1;
//   }

//   *mtx_out = mtx;
//   return 0;
// }

// int ota_osal_mutex_lock(ota_osal_mutex_t *mtx, uint32_t timeout_ms) {
//   TickType_t ticks;

//   if (!mtx) {
//     return -1;
//   }

//   ticks =
//       (timeout_ms == 0xFFFFFFFFU) ? portMAX_DELAY :
//       pdMS_TO_TICKS(timeout_ms);
//   return (xSemaphoreTake(mtx->mutex, ticks) == pdTRUE) ? 0 : -1;
// }

// int ota_osal_mutex_unlock(ota_osal_mutex_t *mtx) {
//   if (!mtx) {
//     return -1;
//   }

//   return (xSemaphoreGive(mtx->mutex) == pdTRUE) ? 0 : -1;
// }

// int ota_osal_mutex_destroy(ota_osal_mutex_t *mtx) {
//   if (!mtx) {
//     return -1;
//   }

//   if (mtx->mutex) {
//     vSemaphoreDelete(mtx->mutex);
//   }

//   free(mtx);
//   return 0;
// }
