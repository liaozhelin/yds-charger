/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-04-07 21:25:11
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-04-07 23:43:35
 * @Description: 
 */
/*

Copyright (c) 2018-2019 Mika Tuupola

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#ifndef _sw_I2C_H
#define _sw_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>

typedef enum{
    SW_I2C_1 = 0,
    SW_I2C_2
}swi2c_t;

void setsw_i2c(swi2c_t i2cselect);

esp_err_t sw_i2c_init(uint8_t sda, uint8_t scl,swi2c_t i2cselect);
esp_err_t sw_i2c_master_start();
esp_err_t sw_i2c_master_stop();

esp_err_t sw_i2c_master_write_byte(uint8_t buffer);
esp_err_t sw_i2c_master_write(uint8_t *buffer, uint8_t length);

esp_err_t sw_i2c_master_read_byte(uint8_t *buffer, bool ack);
esp_err_t sw_i2c_master_read(uint8_t *buffer, uint16_t length, bool ack);

#ifdef __cplusplus
}
#endif

#endif