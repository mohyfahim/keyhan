#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include <stdio.h>

#include "keyhan/agent.h"
#include "keyhan/error.h"
#include "keyhan/ports/espidf_osal.h"

static const char *TAG = "keyhan-esp32-sync-http";
static const char *DEVICE_TOKEN = "abc123";

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < 10) {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI(TAG, "retry to connect to the AP");
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    ESP_LOGI(TAG, "connect to the AP fail");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

static void on_state_changed(int old_state, int new_state, void *user) {
  ESP_LOGI(TAG, "agent state changed: %d -> %d", old_state, new_state);
  (void)user;
}

static void on_progress(uint32_t downloaded, uint32_t total, void *user) {
  (void)user;
  ESP_LOGI(TAG, "download progress: %" PRIu32 "/%" PRIu32, downloaded, total);
}

static void on_error(keyhan_agent_error_t err, void *user) {
  (void)user;
  ESP_LOGE(TAG, "OTA error: %s", KEYHAN_AGENT_ERROR_TO_NAME(err));
}

static void on_update_ready(void *user) {
  (void)user;
  ESP_LOGI(TAG, "update staged and ready");
}

esp_err_t dummy_write_version_to_nvs(nvs_handle_t hndl) {

  int32_t version = 0;
  ESP_LOGI(TAG, "\nWriting app_version to NVS...");
  esp_err_t err = nvs_set_i32(hndl, "app_version", version);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write app_version!, err: %s",
             esp_err_to_name(err));
  }

  return ESP_OK;
}

typedef struct {
  nvs_handle_t nvs;
  uint32_t staged_size;
} app_osal_userdata_t;

static keyhan_agent_error_t app_stage_chunk(void *user, const uint8_t *data,
                                            size_t len) {
  app_osal_userdata_t *ctx = (app_osal_userdata_t *)user;
  (void)data;
  if (!ctx) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }
  ctx->staged_size += (uint32_t)len;
  return KEYHAN_AGENT_OK;
}

static keyhan_agent_error_t app_apply_staged(void *user, int target_version,
                                             int auto_reboot) {
  app_osal_userdata_t *ctx = (app_osal_userdata_t *)user;
  esp_err_t err;
  if (!ctx) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  err = nvs_set_i32(ctx->nvs, "app_version", target_version);
  if (err != ESP_OK || nvs_commit(ctx->nvs) != ESP_OK) {
    return KEYHAN_AGENT_ERR_OS;
  }

  ESP_LOGI(TAG, "applied version=%d staged_size=%" PRIu32, target_version,
           ctx->staged_size);
  if (auto_reboot) {
    esp_restart();
  }
  return KEYHAN_AGENT_OK;
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

  int32_t app_version = -1;
  ESP_LOGI(TAG, "\nReading app version from NVS...");
  err = nvs_get_i32(nvs_handle, "app_version", &app_version);
  switch (err) {
  case ESP_OK:
    ESP_LOGI(TAG, "app version = %" PRIu32, app_version);
    break;
  case ESP_ERR_NVS_NOT_FOUND:
    ESP_LOGW(TAG, "The value is not initialized yet!");
    dummy_write_version_to_nvs(nvs_handle);
    app_version = 0;
    break;
  default:
    ESP_LOGE(TAG, "Error (%s) reading!", esp_err_to_name(err));
    return;
  }

  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = "GolKhoone",
              .password = "76967696",
          },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "wifi_init_sta finished.");

  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we
   * can test which event actually happened. */
  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", "GolKhoone",
             "76967696");
  } else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", "GolKhoone",
             "76967696");
  } else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
  }

  keyhan_agent_t *agent = NULL;
  app_osal_userdata_t app_osal_user = {.nvs = nvs_handle, .staged_size = 0};
  keyhan_espidf_osal_ctx_t osal_ctx = {
      .manifest_url = CONFIG_KEYHAN_TARGET_URI,
      .stage_chunk = app_stage_chunk,
      .apply_staged = app_apply_staged,
      .user = &app_osal_user,
  };

  keyhan_agent_device_info_t devinfo = {
      .current_version = app_version,
      .device_token = DEVICE_TOKEN,
  };
  keyhan_agent_init_params_t params = {
      .auto_apply = true,
      .auto_reboot = false,
      .osal_ops = &g_keyhan_espidf_osal_ops,
      .osal_ctx = &osal_ctx,
  };
  keyhan_agent_callbacks_t cb = {
      .on_state_changed = on_state_changed,
      .on_progress = on_progress,
      .on_error = on_error,
      .on_update_ready = on_update_ready,
      .user = NULL,
  };

  keyhan_agent_error_t k_err;

  k_err = keyhan_agent_init(&agent, &devinfo, &params, &cb);
  if (k_err != KEYHAN_AGENT_OK) {
    ESP_LOGE(TAG, "Error on initializing keyhan agent, err: %s",
             KEYHAN_AGENT_ERROR_TO_NAME(k_err));
    return;
  }

  k_err = keyhan_agent_start(agent);
  if (k_err != KEYHAN_AGENT_OK) {
    ESP_LOGE(TAG, "Error on starting keyhan agent, err: %s",
             KEYHAN_AGENT_ERROR_TO_NAME(k_err));
    return;
  }

  while (true) {
    k_err = keyhan_agent_step(agent);
    if (k_err != KEYHAN_AGENT_OK && k_err != KEYHAN_AGENT_ERR_NO_UPDATE) {
      ESP_LOGE(TAG, "Error on polling keyhan agent: %s",
               KEYHAN_AGENT_ERROR_TO_NAME(k_err));
    }

    ESP_LOGD(TAG, "loop");

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
