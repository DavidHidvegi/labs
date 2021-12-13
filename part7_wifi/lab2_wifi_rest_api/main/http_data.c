/****************************************************************************
 * Copyright (C) 2021 by Fabrice Muller                                     *
 *                                                                          *
 * This file is useful for ESP32 Design course.                             *
 *                                                                          *
 ****************************************************************************/

/**
 * @file http_data.c
 * @author Fabrice Muller
 * @date 16 Nov. 2021
 * @brief File containing the lab1-3 of Part 7.
 *
 * @see https://github.com/fmuller-pns/esp32-vscode-project-template
 */

#include "http_data.h"

static const char *TAG = "HTTP_DATA";

/**
 * @brief The HTTP event handler
 * 
 * @param evt HTTP Client events data
 * @return esp_err_t ESP_OK if no error else ESP_FAIL
 */
esp_err_t wifi_http_event_handler_cb(esp_http_client_event_t *evt) {

  /* Get the http param variable for the buffer */
  http_param_t *http_param = (http_param_t *)evt->user_data;

  switch (evt->event_id) {

    case HTTP_EVENT_ON_CONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
      break;

    case HTTP_EVENT_HEADER_SENT:
      ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
      break;    

    case HTTP_EVENT_ON_HEADER:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
      printf("%.*s", evt->data_len, (char*)evt->data);
      break;  

    case HTTP_EVENT_ON_DATA:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA Len=%d", evt->data_len);

      /* Buffered data */
      if (http_param->buffer == NULL) {
          http_param->buffer = (char *)malloc(evt->data_len);
      }
      else {
          http_param->buffer = (char *)realloc(http_param->buffer, evt->data_len + http_param->index);
      }
      memcpy(&http_param->buffer[http_param->index], evt->data, evt->data_len);
      http_param->index += evt->data_len;

      break;

    case HTTP_EVENT_ON_FINISH:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");

      /* Buffered data */
      http_param->buffer = (char *)realloc(http_param->buffer, http_param->index + 1);
      memcpy(&http_param->buffer[http_param->index], "\0", 1);
      break;

    case HTTP_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
      break;

    case HTTP_EVENT_ERROR:
      ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
      return ESP_FAIL;
      break;

    default:
      ESP_LOGW(TAG, "HTTP_EVENT UNDEFINED EVENT");
      break;

  }
  return ESP_OK;
}

/**
 * @brief Fetch HTTP data (blocking function)
 * 
 * @param http_param empty buffer
 * @param url url to fetch data
 * @return esp_err_t return ESP_OK or ESP_FAIL if error
 */
esp_err_t fetchHttpData(http_param_t *http_param, const char *url) {

  /* Init HTTP paramaters */
  http_param->buffer = NULL;
  http_param->index = 0;

  /* HTTP session init */
  esp_http_client_config_t clientConfig = {
      .url = url,
      .user_data = (void *)http_param,
      .event_handler = wifi_http_event_handler_cb};

  esp_http_client_handle_t client = esp_http_client_init(&clientConfig);

  /* Perform HTTP session in blocking mode. Wait until http session is finished */
  esp_err_t err = esp_http_client_perform(client);

  /* Print information as soon as the session is finished */
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "HTTP GET status = %d, content_length = %d",
              esp_http_client_get_status_code(client),
              esp_http_client_get_content_length(client));
  }
  else {
      ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
  }

  /* Clean the client session */
  esp_http_client_cleanup(client);  

  return (err == ESP_OK ? ESP_OK : ESP_FAIL); 
}
