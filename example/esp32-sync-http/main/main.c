#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <inttypes.h>
#include <stdio.h>

static const char *TAG = "keyhan-esp32-sync-http";

// static void on_state_changed(int old_state, int new_state, void *user) {
//   (void)old_state;
//   (void)new_state;
//   (void)user;
// }

// static void on_progress(uint32_t downloaded, uint32_t total, void *user) {
//   (void)user;
//   // app log/telemetry
// }

// static void on_error(keyhan_agent_error_t err, void *user) {
//   (void)user;
//   // app error handling
// }

// static void on_update_ready(void *user) {
//   (void)user;
//   // trigger apply or wait for maintenance window
// }

esp_err_t dummy_write_version_to_nvs(nvs_handle_t hndl) {

  int32_t version = 1;
  ESP_LOGI(TAG, "\nWriting app_version to NVS...");
  esp_err_t err = nvs_set_i32(hndl, "app_version", version);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write app_version!, err: %s",
             esp_err_to_name(err));
  }

  return ESP_OK;
}

void app_main(void) {

  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  // Open NVS handle
  ESP_LOGI(TAG, "\nOpening Non-Volatile Storage (NVS) handle...");
  nvs_handle_t nvs_handle;
  err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    return;
  }

  int32_t app_version = 0;
  ESP_LOGI(TAG, "\nReading app version from NVS...");
  err = nvs_get_i32(nvs_handle, "app_version", &app_version);
  switch (err) {
  case ESP_OK:
    ESP_LOGI(TAG, "app version = %" PRIu32, app_version);
    break;
  case ESP_ERR_NVS_NOT_FOUND:
    ESP_LOGW(TAG, "The value is not initialized yet!");
    dummy_write_version_to_nvs(nvs_handle);
    break;
  default:
    ESP_LOGE(TAG, "Error (%s) reading!", esp_err_to_name(err));
  }

  // keyhan_agent_t *agent = NULL;

  // keyhan_agent_device_info_t devinfo = {
  //     .current_version = 0,
  //     .device_id = "dev1",
  //     .project_id = "proj1",
  // };
  // keyhan_agent_init_params_t params = {
  //     .auto_apply = false,
  //     .auto_reboot = false,
  // };
  // keyhan_agent_callbacks_t cb = {
  //     .on_state_changed = on_state_changed,
  //     .on_progress = on_progress,
  //     .on_error = on_error,
  //     .on_update_ready = on_update_ready,
  //     .user = NULL,
  // };

  // keyhan_agent_error_t err;

  // err = keyhan_agent_init(&agent, &devinfo, &params, &cb);
  // if (err != KEYHAN_AGENT_OK) {
  //   // todo
  // }

  // err = keyhan_agent_start(agent);
  // if (err != KEYHAN_AGENT_OK) {
  //   // todo
  // }

  while (true) {

    // err = keyhan_agent_step(agent);
    // if (err != KEYHAN_AGENT_OK) {
    //   // todo
    // }

    ESP_LOGD(TAG, "loop");

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
