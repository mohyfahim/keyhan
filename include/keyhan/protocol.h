#pragma once

#include "keyhan/error.h"
#include "keyhan/osal.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KEYHAN_OTA_PROTOCOL_VERSION 1U
#define KEYHAN_OTA_PROTOCOL_FOOTER_MAGIC 0xDEADBEEFU
#define KEYHAN_OTA_PROTOCOL_MAX_BODY_LEN 512U

#define KEYHAN_OTA_PROTOCOL_HEADER_DEVICE_TOKEN "X-Device-Token"
#define KEYHAN_OTA_PROTOCOL_HEADER_ACCEPT "application/octet-stream"

typedef enum {
  KEYHAN_OTA_MSG_TYPE_MANIFEST = 1,
} keyhan_ota_msg_type_t;

typedef struct __attribute__((packed)) {
  uint8_t version;
  uint32_t msg_len; /* little-endian on the wire */
  uint8_t msg_type;
} keyhan_ota_msg_header_t;

typedef struct __attribute__((packed)) {
  int32_t target_version; /* little-endian on the wire */
  uint32_t image_size;    /* little-endian on the wire */
  uint16_t image_url_len; /* little-endian on the wire */
  /* image_url bytes follow (not null-terminated in frame) */
} keyhan_ota_manifest_body_t;

keyhan_agent_error_t
keyhan_protocol_parse_manifest_v1(const uint8_t *payload, size_t payload_len,
                                  keyhan_ota_update_info_t *out_info);

#ifdef __cplusplus
}
#endif
