#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "keyhan/agent.h"
#include <stdio.h>

static void on_state_changed(int old_state, int new_state, void *user) {
  (void)old_state;
  (void)new_state;
  (void)user;
}

static void on_progress(uint32_t downloaded, uint32_t total, void *user) {
  (void)user;
  // app log/telemetry
}

static void on_error(keyhan_agent_error_t err, void *user) {
  (void)user;
  // app error handling
}

static void on_update_ready(void *user) {
  (void)user;
  // trigger apply or wait for maintenance window
}

void app_main(void) {

  keyhan_agent_t *agent = NULL;

  keyhan_agent_device_info_t devinfo = {
      .current_version = 0,
      .device_id = "dev1",
      .project_id = "proj1",
  };
  keyhan_agent_init_params_t params = {
      .auto_apply = false,
      .auto_reboot = false,
  };
  keyhan_agent_callbacks_t cb = {
      .on_state_changed = on_state_changed,
      .on_progress = on_progress,
      .on_error = on_error,
      .on_update_ready = on_update_ready,
      .user = NULL,
  };

  keyhan_agent_error_t err;

  err = keyhan_agent_init(&agent, &devinfo, &params, &cb);
  if (err != KEYHAN_AGENT_OK) {
    // todo
  }

  err = keyhan_agent_start(agent);
  if (err != KEYHAN_AGENT_OK) {
    // todo
  }

  while (true) {

    err = keyhan_agent_step(agent);
    if (err != KEYHAN_AGENT_OK) {
      // todo
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
