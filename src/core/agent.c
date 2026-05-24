#include "keyhan/agent.h"
#include "internal.h"
#include "keyhan/error.h"

#include <stdlib.h>
#include <string.h>

static void keyhan_agent_set_state(keyhan_agent_t *agent,
                                   keyhan_agent_state_t new_state) {
  keyhan_agent_state_t old_state = agent->state;
  agent->state = new_state;
  if (agent->cb && agent->cb->on_state_changed) {
    agent->cb->on_state_changed((int)old_state, (int)new_state, agent->cb->user);
  }
}

static void keyhan_agent_raise_error(keyhan_agent_t *agent,
                                     keyhan_agent_error_t err) {
  keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_ERROR);
  if (agent->cb && agent->cb->on_error) {
    agent->cb->on_error(err, agent->cb->user);
  }
}

keyhan_agent_error_t keyhan_agent_init(keyhan_agent_t **agent_out,
                                       keyhan_agent_device_info_t *devinfo,
                                       keyhan_agent_init_params_t *params,
                                       keyhan_agent_callbacks_t *cb) {

  if (!agent_out || !devinfo || !params || !cb) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }
  keyhan_agent_t *agent;
  agent = (keyhan_agent_t *)calloc(1, sizeof(keyhan_agent_t));
  if (!agent) {
    return KEYHAN_AGENT_ERR_NO_MEM;
  }
  agent->state = KEYHAN_AGENT_STATE_IDLE;
  agent->devinfo = devinfo;
  agent->params = params;
  agent->cb = cb;
  memset(&agent->pending_update, 0, sizeof(agent->pending_update));
  if (!params->osal_ops || !params->osal_ops->fetch_update_info ||
      !params->osal_ops->download_and_stage ||
      !params->osal_ops->apply_staged_update) {
    free(agent);
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  *agent_out = agent;

  return KEYHAN_AGENT_OK;
}

keyhan_agent_error_t keyhan_agent_start(keyhan_agent_t *agent) {

  if (!agent) {
    return KEYHAN_AGENT_ERR_UNINITIALIZED;
  }
  return keyhan_agent_step(agent);
}

keyhan_agent_error_t keyhan_agent_step(keyhan_agent_t *agent) {
  keyhan_agent_error_t err;
  keyhan_ota_update_info_t latest;
  const keyhan_osal_ota_ops_t *ops;

  if (!agent || !agent->params || !agent->params->osal_ops) {
    return KEYHAN_AGENT_ERR_UNINITIALIZED;
  }
  ops = agent->params->osal_ops;
  memset(&latest, 0, sizeof(latest));

  keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_CHECKING);
  err = ops->fetch_update_info(agent->params->osal_ctx, agent->devinfo->device_token,
                               &latest);
  if (err != KEYHAN_AGENT_OK) {
    if (err == KEYHAN_AGENT_ERR_NO_UPDATE) {
      keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_RUNNING);
      return KEYHAN_AGENT_OK;
    }
    keyhan_agent_raise_error(agent, err);
    return err;
  }

  if (latest.version <= agent->devinfo->current_version) {
    keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_RUNNING);
    return KEYHAN_AGENT_OK;
  }

  agent->pending_update = latest;
  keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_DOWNLOADING);
  err = ops->download_and_stage(agent->params->osal_ctx, &latest,
                                (agent->cb ? agent->cb->on_progress : NULL),
                                (agent->cb ? agent->cb->user : NULL));
  if (err != KEYHAN_AGENT_OK) {
    keyhan_agent_raise_error(agent, err);
    return err;
  }

  if (!agent->params->auto_apply) {
    keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_UPDATED);
    if (agent->cb && agent->cb->on_update_ready) {
      agent->cb->on_update_ready(agent->cb->user);
    }
    return KEYHAN_AGENT_OK;
  }

  keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_APPLYING);
  err = ops->apply_staged_update(agent->params->osal_ctx, latest.version,
                                 agent->params->auto_reboot ? 1 : 0);
  if (err != KEYHAN_AGENT_OK) {
    keyhan_agent_raise_error(agent, err);
    return err;
  }

  agent->devinfo->current_version = latest.version;
  keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_UPDATED);
  return KEYHAN_AGENT_OK;
}

keyhan_agent_error_t keyhan_agent_stop(keyhan_agent_t *agent) {
  if (!agent) {
    return KEYHAN_AGENT_ERR_UNINITIALIZED;
  }
  keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_STOPPED);

  return KEYHAN_AGENT_OK;
}

keyhan_agent_error_t keyhan_agent_deinit(keyhan_agent_t *agent) {

  if (agent) {
    free(agent);
  }
  return KEYHAN_AGENT_OK;
}