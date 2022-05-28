/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-02-10 12:07:09
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-04-30 16:28:53
 * @Description: 
 */
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "adc_read.h"
#include "esp_log.h"

static const char * TAG = "adc_read";
static esp_adc_cal_characteristics_t *adc_chars;

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

void ADC_Init(void){
    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channel1, atten1);
        adc1_config_channel_atten(channel2, atten2);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel1, atten1);
    }
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten1, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
}

void ADC_getVoltage(uint32_t *adcdate){
    uint32_t adc_reading[2] = {};
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        if (unit == ADC_UNIT_1) {
            adc_reading[0] += adc1_get_raw((adc1_channel_t)channel1);
            adc_reading[1] += adc1_get_raw((adc1_channel_t)channel2);
        } else {
            int raw;
            adc2_get_raw((adc2_channel_t)channel1, width, &raw);
            adc_reading[0] += raw;
            adc_reading[1] += raw;
        }
    }
    adc_reading[0] /= NO_OF_SAMPLES;
    adc_reading[1] /= NO_OF_SAMPLES;
    //转换 adc_reading 到 voltage(mV)
    *adcdate = esp_adc_cal_raw_to_voltage(adc_reading[0], adc_chars)*72/10;
    *(adcdate+1) = esp_adc_cal_raw_to_voltage(adc_reading[1], adc_chars)*72/10;
    //ESP_LOGI(TAG,"Raw1: %d\tVoltage: %dmV  Raw2: %d\tVoltage: %dmV\n", adc_reading[0], *adcdate, adc_reading[1], *(adcdate+1));
}

// void ADC_getVoltage_task(void *arg){
//     esp_adc_cal_characteristics_t *adc_chars;
//     adc_chars = (esp_adc_cal_characteristics_t *)arg;
//     //Continuously sample ADC1
//     while (1)
//     {
//         uint32_t adc_reading = 0;
//         //多次采样
//         for (int i = 0; i < NO_OF_SAMPLES; i++)
//         {
//             if (unit == ADC_UNIT_1)
//             {
//                 adc_reading += adc1_get_raw((adc1_channel_t)channel);
//             }
//             else
//             {
//                 int raw;
//                 adc2_get_raw((adc2_channel_t)channel, width, &raw);
//                 adc_reading += raw;
//             }
//         }
//         adc_reading /= NO_OF_SAMPLES;
//         //转换 adc_reading 到 voltage(mV)
//         uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
//         ESP_LOGI(TAG,"Raw1: %d\tVoltage: %dmV\n", adc_reading, voltage);
//         ESP_LOGI(TAG,"Raw2: %d\tVoltage: %dmV\n", adc_reading, voltage);
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }