/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-05-10 14:35:47
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-06-14 11:17:14
 * @Description: 
 */
#include "task.h"

#include <stdio.h>
#include <string.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "adc_read.h"
#include "lis3dh.h"
//#include "led_strip.h"
#include "sw3526.h"

static const char *TAG = "task";

void adcTask(void *pvParameters)
{
  for (;;)
  {
    ADC_getVoltage(ADC);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
  vTaskDelete(NULL);
}

void sw35xxTask(void *pvParameters)
{
  for (;;)
  {
    SW35XXUpdate();
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void ws28xxTask(void *pvParameters)
{
  uint32_t red = 0;
  uint32_t green = 0;
  uint32_t blue = 0;
  uint8_t rgb_flag = 0;
  for (;;)
  {
    for (int j = 0; j < 4; j += 1)
    {
      ESP_ERROR_CHECK(strip->set_pixel(strip, j, 0, green, 0));
    }
    ESP_ERROR_CHECK(strip->refresh(strip, 100));
    if (!rgb_flag)
    {
      green = (green > 150 ? 150 : green + 1);
      if (green == 150)
      {
        rgb_flag = 1;
      }
    }
    else
    {
      green = (green < 1 ? 1 : green - 1);
      if (green == 1)
      {
        rgb_flag = 0;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void lis3dhTask(void *pvParameters)
{
  uint8_t buffer1,buffer2;
  uint16_t X_V,Y_V,Z_V;
  for (;;)
  {
    LIS3DH_ReadReg(LIS3DH_REG_OUT_X_L,&buffer1);
    LIS3DH_ReadReg(LIS3DH_REG_OUT_X_H,&buffer2);
    X_V = ((buffer2<<8)|buffer1);
    LIS3DH_ReadReg(LIS3DH_REG_OUT_Y_L,&buffer1);
    LIS3DH_ReadReg(LIS3DH_REG_OUT_Y_H,&buffer2);
    Y_V = ((buffer2<<8)|buffer1);
    LIS3DH_ReadReg(LIS3DH_REG_OUT_Z_L,&buffer1);
    LIS3DH_ReadReg(LIS3DH_REG_OUT_Z_H,&buffer2);
    Z_V = ((buffer2<<8)|buffer1);
    //ESP_LOGI(TAG, "LIS3DH_X=%d  LIS3DH_Y=%d  LIS3DH_Z=%d",X_V,Y_V,Z_V);
    vTaskDelay(pdMS_TO_TICKS(40));
  }
}

void ntpClockTask(void *pvParameters){
  for (;;){
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void taskMonitor(void *pvParameters){
  UBaseType_t uxHighWaterMark;
  for (;;){
    vTaskDelay(pdMS_TO_TICKS(1000));
    printf("--------------------------------------------\r\n");
    uxHighWaterMark = uxTaskGetStackHighWaterMark(adcTask_handle);
    printf("Task: adcTask_handle stacksize=%d\r\n",uxHighWaterMark);

    uxHighWaterMark = uxTaskGetStackHighWaterMark(sw35xxTask_handle);
    printf("Task: sw35xxTask_handle stacksize=%d\r\n",uxHighWaterMark);

    uxHighWaterMark = uxTaskGetStackHighWaterMark(ws28xxTask_handle);
    printf("Task: ws28xxTask_handle stacksize=%d\r\n",uxHighWaterMark);

    uxHighWaterMark = uxTaskGetStackHighWaterMark(lis3dhtask_handle);
    printf("Task: lis3dhtask_handle stacksize=%d\r\n",uxHighWaterMark);

    uxHighWaterMark = uxTaskGetStackHighWaterMark(oledTask_handle);
    printf("Task: oledTask_handle stacksize=%d\r\n",uxHighWaterMark);

    uxHighWaterMark = uxTaskGetStackHighWaterMark(ntpTask_handle);
    printf("Task: ntpTask_handle stacksize=%d\r\n",uxHighWaterMark);
  }
}
