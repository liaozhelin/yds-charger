/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-04-03 10:08:01
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-04-30 16:52:46
 * @Description: 
 */
#ifndef __LIS3DH_C
#define __LIS3DH_C
#include "esp_system.h"

// List of LIS3DH registers
#define LIS3DH_REG_STATUS_AUX   0x07
#define LIS3DH_REG_OUT_ADC1_L   0x08
#define LIS3DH_REG_OUT_ADC1_H   0x09
#define LIS3DH_REG_OUT_ADC2_L   0x0a
#define LIS3DH_REG_OUT_ADC2_H   0x0b
#define LIS3DH_REG_OUT_ADC3_L   0x0c
#define LIS3DH_REG_OUT_ADC3_H   0x0d
#define LIS3DH_REG_INT_COUNTER  0x0e
#define LIS3DH_REG_WHO_AM_I     0x0f
#define LIS3DH_REG_TEMP_CFG     0x1f
#define LIS3DH_REG_CTRL1        0x20
#define LIS3DH_REG_CTRL2        0x21
#define LIS3DH_REG_CTRL3        0x22
#define LIS3DH_REG_CTRL4        0x23
#define LIS3DH_REG_CTRL5        0x24
#define LIS3DH_REG_CTRL6        0x25
#define LIS3DH_REG_REFERENCE    0x26
#define LIS3DH_REG_STATUS       0x27
#define LIS3DH_REG_OUT_X_L      0x28
#define LIS3DH_REG_OUT_X_H      0x29
#define LIS3DH_REG_OUT_Y_L      0x2a
#define LIS3DH_REG_OUT_Y_H      0x2b
#define LIS3DH_REG_OUT_Z_L      0x2c
#define LIS3DH_REG_OUT_Z_H      0x2d
#define LIS3DH_REG_FIFO_CTRL    0x2e
#define LIS3DH_REG_FIFO_SRC     0x2f
#define LIS3DH_REG_INT1_CFG     0x30
#define LIS3DH_REG_INT1_SRC     0x31
#define LIS3DH_REG_INT1_THS     0x32
#define LIS3DH_REG_INT1_DUR     0x33
#define LIS3DH_REG_INT2_CFG     0x34
#define LIS3DH_REG_INT2_SRC     0x35
#define LIS3DH_REG_INT2_THS     0x36
#define LIS3DH_REG_INT2_DUR     0x37
#define LIS3DH_REG_CLICK_CFG    0x38
#define LIS3DH_REG_CLICK_SRC    0x39
#define LIS3DH_REG_CLICK_THS    0x3a
#define LIS3DH_REG_TIME_LIMIT   0x3b
#define LIS3DH_REG_TIME_LATENCY 0x3c
#define LIS3DH_REG_TIME_WINDOW  0x3d

typedef struct
{
    uint8_t highPassFilterCutoff;
    uint8_t highPassFilterMode;
    uint8_t ODR;
} LIS3DH_IRQ_Config_t;

esp_err_t LIS3DH_Init();
esp_err_t LIS3DH_ReadReg(uint8_t reg, uint8_t *readData);
esp_err_t LIS3DH_ConfigureMotionDetection(void);

#endif //__LIS3DH_C
