#include "keyhan/osal.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "internal.h"
#include "keyhan/agent.h"
#include "keyhan/error.h"

#include <stdlib.h>
#include <sys/param.h>

static const char *TAG = "keyhan_osal_espidf";

#define MAX_KEYHAN_OSAL_HTTP_EVENT_BUFFER_SIZE                                 \
  sizeof(keyhan_agent_transport_msg_t)

struct keyhan_osal_transport_client {
#ifdef CONFIG_KEYHAN_TRANSPORT_USE_HTTP
  esp_http_client_handle_t handle;
#elif defined(CONFIG_KEYHAN_TRANSPORT_USE_SOCKET)
  int socket_fd;
#endif
};

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
  static char
      output_buffer[MAX_KEYHAN_OSAL_HTTP_EVENT_BUFFER_SIZE]; // Buffer to store
                                                             // response of http
                                                             // request from
                                                             // event handler
  static int output_len; // Stores number of bytes read
  switch (evt->event_id) {
  case HTTP_EVENT_ERROR:
    ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
    break;
  case HTTP_EVENT_HEADER_SENT:
    ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
    break;
  case HTTP_EVENT_ON_HEADER:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key,
             evt->header_value);
    break;
  case HTTP_EVENT_ON_DATA:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
    // Clean the buffer in case of a new request
    if (!output_len) {
      // we are just starting to copy the output data into the use
      memset(output_buffer, 0, MAX_KEYHAN_OSAL_HTTP_EVENT_BUFFER_SIZE);
    }
    /*
     *  Check for chunked encoding is added as the URL for chunked encoding used
     * in this example returns binary data. However, event handler can also be
     * used in case chunked encoding is used.
     */
    if (!esp_http_client_is_chunked_response(evt->client)) {
      // If user_data buffer is configured, copy the response into the buffer
      int copy_len = 0;
      int content_len = esp_http_client_get_content_length(evt->client);
      copy_len = MIN(evt->data_len, (content_len - output_len));
      if (copy_len + output_len < MAX_KEYHAN_OSAL_HTTP_EVENT_BUFFER_SIZE) {
        memcpy(output_buffer + output_len, evt->data, copy_len);
      } else {
        ESP_LOGW(TAG, "recieved message larger than the protocol: %d",
                 output_len + copy_len);
      }

      output_len += copy_len;
    }

    break;
  case HTTP_EVENT_ON_FINISH: {
    ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
    ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
    keyhan_agent_error_t err = keyhan_utils_fifo_push(
        (keyhan_utils_fifo_t *)evt->user_data, output_buffer);
    if (err != KEYHAN_AGENT_OK) {
      ESP_LOGE(TAG, "Error on pushing to buffer: %s",
               keyhan_agent_error_to_name(err));
    }
    memset(output_buffer, 0, MAX_KEYHAN_OSAL_HTTP_EVENT_BUFFER_SIZE);
    output_len = 0;
    break;
  }

  case HTTP_EVENT_DISCONNECTED: {
    ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
    int mbedtls_err = 0;
    esp_err_t err = esp_tls_get_and_clear_last_error(
        (esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
    if (err != 0) {
      ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
      ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
    }
    memset(output_buffer, 0, MAX_KEYHAN_OSAL_HTTP_EVENT_BUFFER_SIZE);
    output_len = 0;
    break;
  }
  case HTTP_EVENT_REDIRECT:
    ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
    break;
  default:
    break;
  }
  return ESP_OK;
}

keyhan_agent_error_t keyhan_osal_transport_http_init(keyhan_agent_t *agent) {

  if (agent->client == NULL) {
    agent->client = malloc(sizeof(struct keyhan_osal_transport_client));
    if (agent->client == NULL) {
      ESP_LOGE(TAG, "Failed to allocate memory for transport client");
      return KEYHAN_AGENT_ERR_UNINITIALIZED; // Or appropriate error
    }
  }

#ifdef CONFIG_KEYHAN_TRANSPORT_USE_HTTP
  esp_http_client_config_t config = {
      .url = CONFIG_KEYHAN_TARGET_URI,
      .event_handler = _http_event_handler,
      .user_data = agent->buffer,
      .disable_auto_redirect = true,
  };

  esp_http_client_handle_t client = esp_http_client_init(&config);
  agent->client->handle = client;

#elif defined(CONFIG_KEYHAN_TRANSPORT_USE_SOCKET)
  int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (fd < 0) {
    ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    return ESP_FAIL;
  }
  agent->client = fd;
#endif
  return KEYHAN_AGENT_OK;
}

keyhan_agent_error_t keyhan_osal_transport_http_post(keyhan_agent_t *agent) {
  if (!agent->client || !agent->client->handle) { // Check both
    return KEYHAN_AGENT_ERR_UNINITIALIZED;
  }
  const char *post_data = "{\"field1\":\"value1\"}";
  esp_http_client_set_method(agent->client->handle, HTTP_METHOD_POST);
  esp_http_client_set_header(agent->client->handle, "Content-Type",
                             "application/keyhan");
  esp_http_client_set_post_field(agent->client->handle, post_data,
                                 strlen(post_data));
  esp_err_t err = esp_http_client_perform(agent->client->handle);
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %" PRId64,
             esp_http_client_get_status_code(agent->client->handle),
             esp_http_client_get_content_length(agent->client->handle));
  } else {
    ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
  }
  return KEYHAN_AGENT_OK;
}
keyhan_agent_error_t keyhan_osal_transport_http_get(keyhan_agent_t *agent) {
  if (!agent->client) {
    return KEYHAN_AGENT_ERR_UNINITIALIZED;
  }

  return KEYHAN_AGENT_OK;
}
keyhan_agent_error_t keyhan_osal_transport_http_deinit(keyhan_agent_t *agent) {
  if (!agent->client) {
    return KEYHAN_AGENT_ERR_UNINITIALIZED;
  }

  return KEYHAN_AGENT_OK;
}