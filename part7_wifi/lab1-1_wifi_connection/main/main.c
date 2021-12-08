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

#include "wifi_connect.h"

static const char *TAG = "WIFI_LAB";
void wifiTask(void *pvParameters);

/**
 * @brief Starting point function
 * 
 */

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
  xSemaphoreHandle wifiSemaphore = getConnectionWifiSemaphore();
  if(xSemaphoreTake(wifiSemaphore, pdMS_TO_TICKS(20000) ) == pdTRUE ){
    printf("connected on %s\n", WIFI_SSID);
    printf("run application\n");
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