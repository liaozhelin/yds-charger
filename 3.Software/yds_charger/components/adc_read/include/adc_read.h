/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-04-07 21:25:11
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-04-29 21:33:30
 * @Description: 
 */
/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-02-10 12:07:09
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-04-07 22:25:36
 * @Description: 
 */
#ifndef _ADC_READ_H_
#define _ADC_READ_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

static const adc_unit_t unit = ADC_UNIT_1;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;

static const adc_atten_t atten1 = ADC_ATTEN_DB_11;
static const adc_channel_t channel1 = ADC_CHANNEL_0; 

static const adc_atten_t atten2 = ADC_ATTEN_DB_11;
static const adc_channel_t channel2 = ADC_CHANNEL_1; 

#define DEFAULT_VREF    1100
#define NO_OF_SAMPLES   64

void ADC_Init(void);
void ADC_getVoltage(uint32_t *adcdate);
// void ADC_getVoltage_task(void *arg);

uint32_t ADC[2];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

