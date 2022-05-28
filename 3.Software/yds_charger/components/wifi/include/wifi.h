/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-02-10 16:05:01
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-04-03 10:12:22
 * @Description: 
 */
#ifndef _WIFI_H_
#define _WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "stdio.h"

#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

uint8_t wifi_init_sta(uint8_t mode);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif