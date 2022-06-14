/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-05-10 14:35:54
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-06-14 11:00:55
 * @Description: 
 */
#ifndef TASK_H
#define TASK_H

#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define ota_process if(OTA_FLAG){xTaskCreate(advanced_ota_example_task, "advanced_ota_example_task", 1024 * 8, NULL, 1, NULL);for (;;){vTaskDelay(pdMS_TO_TICKS(1000));}}
#define ota_set OTA_FLAG = 1;

bool OTA_FLAG;


void adcTask(void *pvParameters);
void sw35xxTask(void *pvParameters);
void ws28xxTask(void *pvParameters);
void lis3dhTask(void *pvParameters);
void ntpClockTask(void *pvParameters);
void taskMonitor(void *pvParameters);

TaskHandle_t adcTask_handle;
TaskHandle_t sw35xxTask_handle;
TaskHandle_t ws28xxTask_handle;
TaskHandle_t lis3dhtask_handle;
TaskHandle_t oledTask_handle;
TaskHandle_t ntpTask_handle;

led_strip_t *strip;

typedef struct{
    struct{
        uint16_t C1;
        uint16_t C2;
    }cPowerLimit;
}YDS;

YDS ysd_t;

#endif