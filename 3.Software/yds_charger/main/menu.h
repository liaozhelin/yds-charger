/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-04-29 20:35:49
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-05-12 00:38:34
 * @Description: 
 */
#ifndef MENU_H
#define MENU_H

#include <stdio.h>

void oledTask(void *pvParameters);
void oledInitMessageTask(uint8_t num,char *state);

#endif