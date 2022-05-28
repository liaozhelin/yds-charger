/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-05-10 14:35:47
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-05-12 21:52:03
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
    ESP_LOGI(TAG, "LIS3DH_X=%d  LIS3DH_Y=%d  LIS3DH_Z=%d",X_V,Y_V,Z_V);
    vTaskDelay(pdMS_TO_TICKS(40));
  }
}