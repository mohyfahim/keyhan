#include "keyhan/agent.h"
#include "internal.h"
#include "keyhan/error.h"
#include "keyhan/osal.h"

#include <stdlib.h>
#include <string.h>

static void keyhan_agent_set_state(keyhan_agent_t *agent,
                                   keyhan_agent_state_t new_state) {
  keyhan_agent_state_t old_state = agent->state;
  agent->state = new_state;
  if (agent->cfg.events.on_state_changed) {
    agent->cfg.events.on_state_changed(old_state, new_state,
                                       agent->cfg.events.user);
  }
}

static void keyhan_agent_raise_error(keyhan_agent_t *agent,
                                     keyhan_agent_error_t err) {
  keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_ERROR);
  if (agent->cfg.events.on_error) {
    agent->cfg.events.on_error(err, agent->cfg.events.user);
  }
}

keyhan_agent_error_t keyhan_agent_init(keyhan_agent_t **agent_out,
                                       const keyhan_agent_config_t *cfg) {

  if (!agent_out || !cfg) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }
  keyhan_agent_t *agent;
  agent = (keyhan_agent_t *)calloc(1, sizeof(keyhan_agent_t));
  if (!agent) {
    return KEYHAN_AGENT_ERR_NO_MEM;
  }
  agent->state = KEYHAN_AGENT_STATE_IDLE;
  memcpy(&agent->cfg, cfg, sizeof(keyhan_agent_config_t));
  memset(&agent->pending, 0, sizeof(keyhan_agent_update_t));

  agent->osal_ops =
      cfg->osal_ops_override ? cfg->osal_ops_override : &g_keyhan_osal_ops;

  *agent_out = agent;

  return KEYHAN_AGENT_OK;
}

keyhan_agent_error_t keyhan_agent_start(keyhan_agent_t *agent) {

  if (!agent) {
    return KEYHAN_AGENT_ERR_UNINITIALIZED;
  }
  keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_STARTING);
  return keyhan_agent_step(agent);
}

keyhan_agent_error_t keyhan_agent_step(keyhan_agent_t *agent) {
  keyhan_agent_error_t err;
  keyhan_agent_update_t latest;
  const keyhan_osal_ota_ops_t *ops;

  if (!agent) {
    return KEYHAN_AGENT_ERR_UNINITIALIZED;
  }
  ops = agent->osal_ops;
  memset(&latest, 0, sizeof(latest));

  keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_CHECKING);
  err = ops->fetch_update_info(agent->cfg.manifest_url, agent->cfg.device_token,
                               &latest);
  if (err != KEYHAN_AGENT_OK) {
    if (err == KEYHAN_AGENT_ERR_NO_UPDATE) {
      keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_RUNNING);
      return KEYHAN_AGENT_OK;
    }
    keyhan_agent_raise_error(agent, err);
    return err;
  }

  if (latest.version <= agent->cfg.current_version) {
    keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_RUNNING);
    return KEYHAN_AGENT_OK;
  }

  agent->pending = latest;
  keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_DOWNLOADING);
  err = ops->download_and_stage(
      &latest,
      (agent->cfg.events.on_progress ? agent->cfg.events.on_progress : NULL),
      (agent->cfg.events.user ? agent->cfg.events.user : NULL));
  if (err != KEYHAN_AGENT_OK) {
    keyhan_agent_raise_error(agent, err);
    return err;
  }

  if (!agent->cfg.auto_apply) {
    keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_UPDATED);
    if (agent->cfg.events.on_update_ready) {
      agent->cfg.events.on_update_ready(agent->cfg.events.user);
    }
    return KEYHAN_AGENT_OK;
  }

  keyhan_agent_set_state(agent, KEYHAN_AGENT_STATE_APPLYING);
  err =
      ops->apply_staged_update(latest.version, agent->cfg.auto_reboot ? 1 : 0);
  if (err != KEYHAN_AGENT_OK) {
    keyhan_agent_raise_error(agent, err);
    return err;
  }

  agent->cfg.current_version = latest.version;
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