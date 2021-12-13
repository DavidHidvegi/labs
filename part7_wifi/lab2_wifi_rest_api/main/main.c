/****************************************************************************
 * Copyright (C) 2021 by Fabrice Muller                                     *
 *                                                                          *
 * This file is useful for ESP32 Design course.                             *
 *                                                                          *
 ****************************************************************************/

/**
 * @file main.c
 * @author Fabrice Muller
 * @date 12 Nov. 2021
 * @brief File containing the lab1-1 of Part 7.
 *
 * @see https://github.com/fmuller-pns/esp32-vscode-project-template
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "http_data.h"

#include "wifi_connect.h"

static const char *TAG = "WIFI_LAB";

#include "esp_http_client.h"
 
void wifiTask(void *pvParameters);

#include "cJSON.h"

/* openweathermap API URL for Cannes city, Unit = degree */
const char *CITY = "Cannes";
const char *OPEN_WEATHER_MAP_URL = "http://api.openweathermap.org/data/2.5/weather?q=Cannes&appid=bfaf90865d45e39c390da17ffa61e195";

/* Example of response for testing the extractJSONWeatherMapInformation() function */
const char *RESP_EXAMPLE = "{\"coord\":{\"lon\":7.0167,\"lat\":43.55},\"weather\":[{\"id\":800,\"main\":\"Clear\",\"description\":\"clear sky\",\"icon\":\"01d\"}],\"base\":\"stations\",\"main\":{\"temp\":24.72,\"feels_like\":24.84,\"temp_min\":23.12,\"temp_max\":25.74,\"pressure\":1019,\"humidity\":61},\"visibility\":10000,\"wind\":{\"speed\":3.6,\"deg\":170},\"clouds\":{\"all\":0},\"dt\":1633099464,\"sys\":{\"type\":1,\"id\":6507,\"country\":\"FR\",\"sunrise\":1633066158,\"sunset\":1633108421},\"timezone\":7200,\"id\":6446684,\"name\":\"Cannes\",\"cod\":200}";

/* Sensor information */
# define WEATHERMAPINFO_DESCRIPTION_LENGTH 100

typedef struct {
  float latitude;
  float longitude;
  float temp;
  float feels_like;
  float temp_min;
  float temp_max;
  float pressure;
  float humidity;
  char description[WEATHERMAPINFO_DESCRIPTION_LENGTH];

} weathermapinfo_t;

void extractJSONWeatherMapInformation(char *resp, weathermapinfo_t *weathermapinfo) {

    /* Convert textual resp to JSON object */
    cJSON *payload = cJSON_Parse(resp);

    /* Coordonate (JSon Items)43.550000,7.016700 */
    cJSON *coord = cJSON_GetObjectItem(payload, "coord");   
    cJSON *longitude = cJSON_GetObjectItem(coord, "lon");
    cJSON *latitude = cJSON_GetObjectItem(coord, "lat");

    cJSON *main_data = cJSON_GetObjectItem(payload, "main");
    cJSON *temp = cJSON_GetObjectItem(main_data, "temp");
    cJSON *feels_like = cJSON_GetObjectItem(main_data, "feels_like");
    cJSON *temp_min = cJSON_GetObjectItem(main_data, "temp_min");
    cJSON *temp_max = cJSON_GetObjectItem(main_data, "temp_max");
    cJSON *pressure = cJSON_GetObjectItem(main_data, "pressure");
    cJSON *humidity = cJSON_GetObjectItem(main_data, "humidity");

    /* Set information in the structure */
    weathermapinfo->latitude = latitude->valuedouble;
    weathermapinfo->longitude = longitude->valuedouble;   
    weathermapinfo->temp = temp->valuedouble;
    weathermapinfo->feels_like = feels_like->valuedouble;
    weathermapinfo->temp_min = temp_min->valuedouble;
    weathermapinfo->temp_max = temp_max->valuedouble;
    weathermapinfo->pressure = pressure->valuedouble;
    weathermapinfo->humidity = humidity->valuedouble;

    /* Free memory */
    cJSON_Delete(payload);
}

void app_main() {
  /* ERROR, WARNING, INFO level log */
  esp_log_level_set(TAG, ESP_LOG_INFO);
  
  /* Init WiFi */
  wifiInit();

  /* Create connected WiFi Task, STACK=3*1024, Priority=5 */
  xTaskCreate( wifiTask, "wifi_main", 3*1024, NULL, 5, NULL);

  /* Delete task */
  vTaskDelete(NULL);
}

void wifiTask(void *pvParameters){

  http_param_t params = {.buffer=NULL, .index=0};
  char *url = malloc(strlen(OPEN_WEATHER_MAP_URL) + strlen(CITY) + 1);
  sprintf(url, OPEN_WEATHER_MAP_URL, CITY);

  xSemaphoreHandle wifiSemaphore = getConnectionWifiSemaphore();
  if(xSemaphoreTake(wifiSemaphore, pdMS_TO_TICKS(10000) ) == pdTRUE ){
    printf("connected on %s\n", WIFI_SSID);
    printf("run application\n");
    fetchHttpData(&params, url);

    /* Extract openweathermap information from response */
    weathermapinfo_t weathermapinfo;
    extractJSONWeatherMapInformation(params.buffer, &weathermapinfo);

    printf("METEO at %s\n", CITY);
    printf("(latitude,longitude) = (%f,%f)\n", weathermapinfo.latitude, weathermapinfo.longitude);
    printf("(temp,feels_like) = (%f,%f)\n", weathermapinfo.temp, weathermapinfo.feels_like);
    printf("(temp_min,temp_max) = (%f,%f)\n", weathermapinfo.temp_min, weathermapinfo.temp_max);
    printf("(pressure,humidity) = (%f,%f)\n", weathermapinfo.pressure, weathermapinfo.humidity);

    vTaskDelay(pdMS_TO_TICKS(2000));
    if (params.buffer != NULL){
      printf("\nBuffered data:\n%.*s\n", params.index, params.buffer);
      free(params.buffer);
      free(url);
    }
    if(xSemaphoreTake(wifiSemaphore,portMAX_DELAY))
      printf("retried connection on %s\n", WIFI_SSID);
  }
  else{
    printf("failed to connect;\n retry in\n");
    for (int i = 0; i < 5; i++)
    {
        printf("%d\n",4-i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    esp_restart();
  }
}