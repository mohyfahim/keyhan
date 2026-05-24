#include "keyhan/protocol.h"

#include <string.h>

static uint16_t keyhan_read_le16(const uint8_t *p) {
  return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t keyhan_read_le32(const uint8_t *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

keyhan_agent_error_t
keyhan_protocol_parse_manifest_v1(const uint8_t *payload, size_t payload_len,
                                  keyhan_ota_update_info_t *out_info) {
  uint8_t version;
  uint32_t msg_len;
  uint8_t msg_type;
  size_t cursor = 0;
  int32_t target_version;
  uint32_t image_size;
  uint16_t image_url_len;
  uint32_t footer_magic = 0;

  if (!payload || payload_len == 0 || !out_info) {
    return KEYHAN_AGENT_ERR_INVALID_ARG;
  }

  if (payload_len < 6U + sizeof(uint32_t)) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }

  version = payload[cursor++];
  msg_len = keyhan_read_le32(payload + cursor);
  cursor += 4;
  msg_type = payload[cursor++];
  if (version != KEYHAN_OTA_PROTOCOL_VERSION) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }
  if (msg_type != KEYHAN_OTA_MSG_TYPE_MANIFEST) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }
  if (msg_len > KEYHAN_OTA_PROTOCOL_MAX_BODY_LEN) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }

  if (payload_len != 6U + (size_t)msg_len + sizeof(uint32_t)) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }
  if ((size_t)msg_len < 10U) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }

  target_version = (int32_t)keyhan_read_le32(payload + cursor);
  cursor += 4;
  image_size = keyhan_read_le32(payload + cursor);
  cursor += 4;
  image_url_len = keyhan_read_le16(payload + cursor);
  cursor += 2;

  if ((size_t)image_url_len + 10U != (size_t)msg_len) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }
  if (image_url_len == 0 || image_url_len >= sizeof(out_info->image_url)) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }

  memset(out_info, 0, sizeof(*out_info));
  out_info->version = target_version;
  out_info->image_size = image_size;
  memcpy(out_info->image_url, payload + cursor, image_url_len);
  out_info->image_url[image_url_len] = '\0';
  cursor += image_url_len;

  memcpy(&footer_magic, payload + cursor, sizeof(footer_magic));
  if (footer_magic != KEYHAN_OTA_PROTOCOL_FOOTER_MAGIC) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }
  if (out_info->version < 0) {
    return KEYHAN_AGENT_ERR_INTEGRITY;
  }

  return KEYHAN_AGENT_OK;
}
