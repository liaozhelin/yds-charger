/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-04-29 20:32:29
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-06-14 14:06:11
 * @Description:
 */
#include <time.h>
#include <sys/time.h>

#include "menu.h"
#include "u8g2_esp32_hal.h"
#include "adc_read.h"
#include "sw3526.h"
#include "esp_log.h"

#include "esp_netif_ip_addr.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "wifi.h"
#include <tcpip_adapter.h>

#include "task.h"
#include "ota.h"
#include "wifi.h"

#define EXIT_MENU_CHECK \
  if (exit_flag)        \
  {                     \
    exit_flag = 0;      \
    break;              \
  }
#define EXIT_MENU_SET exit_flag = 1;

static const char *TAG = "memu";

static void oledWifiShowTask(void)
{
  wifi_ap_record_t ap;
  tcpip_adapter_ip_info_t ipInfo;
  for (;;)
  {
    char buf[20] = {0};
    esp_wifi_sta_get_ap_info(&ap);
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
    uint8_t event = u8x8_GetMenuEvent(u8g2_GetU8x8(&u8g2));
    u8g2_ClearBuffer(&u8g2);
    sprintf(buf, "SSID名称: %s", ap.ssid);
    u8g2_DrawUTF8(&u8g2, 0, 15, buf);
    sprintf(buf, "IP地址: " IPSTR, IP2STR(&ipInfo.ip));
    u8g2_DrawUTF8(&u8g2, 0, 31, buf);
    sprintf(buf, "RSSI信号强度: %d", ap.rssi);
    u8g2_DrawUTF8(&u8g2, 0, 47, buf);
    sprintf(buf, "Primary信道: %d", ap.primary);
    u8g2_DrawUTF8(&u8g2, 0, 63, buf);
    u8g2_SendBuffer(&u8g2);
    if (event == U8X8_MSG_GPIO_MENU_SELECT)
    {
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void oledSmartConfigTask(void){
    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawUTF8(&u8g2, 20, 16, "已进入配网模式");
    u8g2_DrawUTF8(&u8g2, 0, 30, "请使用ESPTouch软件");
    u8g2_DrawUTF8(&u8g2, 0, 46, "只支持2.4G频段 WIFI");
    u8g2_DrawUTF8(&u8g2, 0, 62, "配网完成后自动退出");
    u8g2_SendBuffer(&u8g2);
    vTaskSuspend(adcTask_handle); //挂起其他任务
    vTaskSuspend(sw35xxTask_handle);
    vTaskSuspend(ws28xxTask_handle);
    vTaskSuspend(lis3dhtask_handle);
    vTaskDelay(pdMS_TO_TICKS(1000));
    wifi_init_sta(1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    vTaskResume(adcTask_handle);
    vTaskResume(sw35xxTask_handle);
    vTaskResume(ws28xxTask_handle);
    vTaskResume(lis3dhtask_handle);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

static void  oledPowerLimitTask(void){
  uint8_t current_selection = 0;
  current_selection = u8g2_UserInterfaceMessage(&u8g2, "C口功率限制设定", "C1口功率:", "C1口功率:", "更新\n取消");
}

static void oledVIShowTask(void)
{
  for (;;)
  {
    char buf[20] = {0};
    uint8_t event = u8x8_GetMenuEvent(u8g2_GetU8x8(&u8g2));
    u8g2_ClearBuffer(&u8g2);
    sprintf(buf, "C1口 %d.%03dV %d.%03dA", (sw35xx_c1.OutVol * 6) / 1000, (sw35xx_c1.OutVol * 6) % 1000, (sw35xx_c1.OutCur * 25 / 10) / 1000, (sw35xx_c1.OutCur * 25 / 10) % 1000);
    u8g2_DrawUTF8(&u8g2, 0, 16, buf);
    sprintf(buf, "C2口 %d.%03dV %d.%03dA", (sw35xx_c2.OutVol * 6) / 1000, (sw35xx_c2.OutVol * 6) % 1000, (sw35xx_c2.OutCur * 25 / 10) / 1000, (sw35xx_c2.OutCur * 25 / 10) % 1000);
    u8g2_DrawUTF8(&u8g2, 0, 32, buf);
    sprintf(buf, "A1口 %d.%03dV", (ADC[0]) / 1000, (ADC[0]) % 1000);
    u8g2_DrawUTF8(&u8g2, 0, 48, buf);
    sprintf(buf, "A2口 %d.%03dV", (ADC[1]) / 1000, (ADC[1]) % 1000);
    u8g2_DrawUTF8(&u8g2, 0, 64, buf);
    u8g2_DrawUTF8(&u8g2, 90, 48, "VIN");
    sprintf(buf, "%d.%03dV", (sw35xx_c1.InVol) / 100, (sw35xx_c1.InVol) % 100);
    u8g2_DrawUTF8(&u8g2, 80, 64, buf);
    u8g2_SendBuffer(&u8g2);
    if (event == U8X8_MSG_GPIO_MENU_SELECT)
    {
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void oledProtocolShowTask_1(void)
{
  char buf[20] = {0};
  u8g2_ClearBuffer(&u8g2);
  u8g2_DrawUTF8(&u8g2, 25, 16, "C1口");
  switch (sw35xx_c1.protocol)
  {
  case 0:
    u8g2_DrawUTF8(&u8g2, 2, 32, "无协议");
    break;
  case 1:
    u8g2_DrawUTF8(&u8g2, 2, 32, "QC 2.0");
    break;
  case 2:
    u8g2_DrawUTF8(&u8g2, 2, 32, "QC 3.0");
    break;
  case 3:
    u8g2_DrawUTF8(&u8g2, 2, 32, "FCP");
    break;
  case 4:
    u8g2_DrawUTF8(&u8g2, 2, 32, "SCP");
    break;
  case 5:
    u8g2_DrawUTF8(&u8g2, 2, 32, "PD FIX");
    break;
  case 6:
    u8g2_DrawUTF8(&u8g2, 2, 32, "PD PPS");
    break;
  case 7:
    u8g2_DrawUTF8(&u8g2, 2, 32, "PE 1.1");
    break;
  case 8:
    u8g2_DrawUTF8(&u8g2, 2, 32, "PE 2.0");
    break;
  case 9:
    u8g2_DrawUTF8(&u8g2, 2, 32, "VOOC");
    break;
  case 10:
    u8g2_DrawUTF8(&u8g2, 2, 32, "SFCP");
    break;
  case 11:
    u8g2_DrawUTF8(&u8g2, 2, 32, "AFC");
    break;
  }
  switch (sw35xx_c1.pdversion)
  {
  case 0:
    break;
  case 1:
    u8g2_DrawUTF8(&u8g2, 2, 48, "PD 2.0");
    break;
  case 2:
    u8g2_DrawUTF8(&u8g2, 2, 48, "PD 3.0");
    break;
  }
  u8g2_DrawLine(&u8g2, 72, 0, 72, 49);
  u8g2_DrawLine(&u8g2, 0, 49, 127, 49);
  u8g2_DrawUTF8(&u8g2, 75, 31, "降压");
  if (sw35xx_c1.state.buck_open)
    u8g2_DrawUTF8(&u8g2, 102, 31, "工作");
  else
    u8g2_DrawUTF8(&u8g2, 102, 31, "停止");
  sprintf(buf, "输出 %d.%03dV %d.%03dA", (sw35xx_c1.OutVol * 6) / 1000, (sw35xx_c1.OutVol * 6) % 1000, (sw35xx_c1.OutCur * 25 / 10) / 1000, (sw35xx_c1.OutCur * 25 / 10) % 1000);
  u8g2_DrawUTF8(&u8g2, 0, 64, buf);
  u8g2_SendBuffer(&u8g2);
}

static void oledProtocolShowTask_2(void)
{
  char buf[20] = {0};
  u8g2_ClearBuffer(&u8g2);
  u8g2_DrawUTF8(&u8g2, 25, 16, "C2口");
  switch (sw35xx_c2.protocol)
  {
  case 0:
    u8g2_DrawUTF8(&u8g2, 2, 32, "无协议");
    break;
  case 1:
    u8g2_DrawUTF8(&u8g2, 2, 32, "QC 2.0");
    break;
  case 2:
    u8g2_DrawUTF8(&u8g2, 2, 32, "QC 3.0");
    break;
  case 3:
    u8g2_DrawUTF8(&u8g2, 2, 32, "FCP");
    break;
  case 4:
    u8g2_DrawUTF8(&u8g2, 2, 32, "SCP");
    break;
  case 5:
    u8g2_DrawUTF8(&u8g2, 2, 32, "PD FIX");
    break;
  case 6:
    u8g2_DrawUTF8(&u8g2, 2, 32, "PD PPS");
    break;
  case 7:
    u8g2_DrawUTF8(&u8g2, 2, 32, "PE 1.1");
    break;
  case 8:
    u8g2_DrawUTF8(&u8g2, 2, 32, "PE 2.0");
    break;
  case 9:
    u8g2_DrawUTF8(&u8g2, 2, 32, "VOOC");
    break;
  case 10:
    u8g2_DrawUTF8(&u8g2, 2, 32, "SFCP");
    break;
  case 11:
    u8g2_DrawUTF8(&u8g2, 2, 32, "AFC");
    break;
  }
  switch (sw35xx_c2.pdversion)
  {
  case 0:
    break;
  case 1:
    u8g2_DrawUTF8(&u8g2, 2, 48, "PD 2.0");
    break;
  case 2:
    u8g2_DrawUTF8(&u8g2, 2, 48, "PD 3.0");
    break;
  }
  u8g2_DrawLine(&u8g2, 72, 0, 72, 49);
  u8g2_DrawLine(&u8g2, 0, 49, 127, 49);
  u8g2_DrawUTF8(&u8g2, 75, 31, "降压");
  if (sw35xx_c2.state.buck_open)
    u8g2_DrawUTF8(&u8g2, 102, 31, "工作");
  else
    u8g2_DrawUTF8(&u8g2, 102, 31, "停止");
  sprintf(buf, "输出 %d.%03dV %d.%03dA", (sw35xx_c2.OutVol * 6) / 1000, (sw35xx_c2.OutVol * 6) % 1000, (sw35xx_c2.OutCur * 25 / 10) / 1000, (sw35xx_c2.OutCur * 25 / 10) % 1000);
  u8g2_DrawUTF8(&u8g2, 0, 64, buf);
  u8g2_SendBuffer(&u8g2);
}

static void oledProtocolShowTask(void)
{
  uint8_t tasknum = 0;
  for (;;)
  {
    uint8_t event = u8x8_GetMenuEvent(u8g2_GetU8x8(&u8g2));
    if (event == U8X8_MSG_GPIO_MENU_SELECT)
    {
      break;
    }
    else if (event == U8X8_MSG_GPIO_MENU_NEXT || event == U8X8_MSG_GPIO_MENU_DOWN)
    {
      tasknum = (tasknum >= 1 ? 1 : tasknum + 1);
    }
    else if (event == U8X8_MSG_GPIO_MENU_PREV || event == U8X8_MSG_GPIO_MENU_UP)
    {
      tasknum = (tasknum <= 0 ? 0 : tasknum - 1);
    }
    switch (tasknum)
    {
    case 0:
      oledProtocolShowTask_1();
      break;
    case 1:
      oledProtocolShowTask_2();
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void oledFaultShowTask(void)
{

  for (;;)
  {
    uint8_t event = u8x8_GetMenuEvent(u8g2_GetU8x8(&u8g2));
    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawUTF8(&u8g2, 0, 15, "C1异常代码");
    u8g2_DrawUTF8(&u8g2, 20, 31, "系统无故障");
    if (sw35xx_c1.state.val_input_upmax)
      u8g2_DrawUTF8(&u8g2, 20, 31, "输入电压过高！");
    if (sw35xx_c1.state.tem_alarm_upmax)
      u8g2_DrawUTF8(&u8g2, 20, 31, "芯片温度过高！");
    if (sw35xx_c1.state.tem_shutdown)
      u8g2_DrawUTF8(&u8g2, 20, 31, "高温关机保护！");
    if (sw35xx_c1.state.short_citcuit)
      u8g2_DrawUTF8(&u8g2, 20, 31, "输出短路！"); // 0x05版本的芯片貌似不能用
    u8g2_DrawUTF8(&u8g2, 0, 47, "C2异常代码");
    u8g2_DrawUTF8(&u8g2, 20, 63, "系统无故障");
    if (sw35xx_c2.state.val_input_upmax)
      u8g2_DrawUTF8(&u8g2, 20, 63, "输入电压过高！");
    if (sw35xx_c2.state.tem_alarm_upmax)
      u8g2_DrawUTF8(&u8g2, 20, 63, "芯片温度过高！");
    if (sw35xx_c2.state.tem_shutdown)
      u8g2_DrawUTF8(&u8g2, 20, 63, "高温关机保护！");
    if (sw35xx_c2.state.short_citcuit)
      u8g2_DrawUTF8(&u8g2, 20, 63, "输出短路！"); // 0x05版本的芯片貌似不能用
    u8g2_SendBuffer(&u8g2);
    if (event == U8X8_MSG_GPIO_MENU_SELECT)
    {
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void oledManualOTATask(void)
{
  uint8_t current_selection = 0;
  current_selection = u8g2_UserInterfaceMessage(&u8g2, "当前: V1.0.0.1", "远端: V1.0.0.2", "有新固件!", "更新\n取消");
  if(current_selection == 1){
    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawUTF8(&u8g2, 15, 16, "设备将会自动更新");
    u8g2_DrawUTF8(&u8g2, 15, 32, "更新完成自动重启");
    u8g2_DrawUTF8(&u8g2, 15, 48, "请耐心等待.....");
    u8g2_SendBuffer(&u8g2);
    vTaskSuspend(adcTask_handle); //挂起其他任务
    vTaskSuspend(sw35xxTask_handle);
    vTaskSuspend(ws28xxTask_handle);
    vTaskSuspend(lis3dhtask_handle);
    vTaskDelay(pdMS_TO_TICKS(1000));
    ota_set
    vTaskDelete(NULL);
    //删除任务，等待复位
  }
}

static void oledAutoOTATask(void)
{
  
}

static void oledChargeSurface(void)
{
  char buf[20] = {0};
  u8g2_ClearBuffer(&u8g2);
  u8g2_DrawLine(&u8g2, 85, 6, 85, 28);
  u8g2_DrawLine(&u8g2, 84, 6, 84, 28);
  u8g2_DrawLine(&u8g2, 85, 40, 85, 62);
  u8g2_DrawLine(&u8g2, 84, 40, 84, 62);

  u8g2_DrawLine(&u8g2, 40, 27, 85, 27);
  u8g2_DrawLine(&u8g2, 40, 28, 85, 28);
  u8g2_DrawLine(&u8g2, 40, 61, 85, 61);
  u8g2_DrawLine(&u8g2, 40, 62, 85, 62);
  u8g2_DrawRFrame(&u8g2, 13, 1, 28, 28, 10);
  u8g2_DrawRFrame(&u8g2, 13, 35, 28, 28, 10);
  u8g2_SetFont(&u8g2, u8g2_font_helvB14_tr);
  sprintf(buf, "%2dW", (sw35xx_c1.OutVol * 6) * (sw35xx_c1.OutCur * 25 / 10) / 1000000);
  u8g2_DrawStr(&u8g2, 43, 22, buf);
  sprintf(buf, "%2dW", (sw35xx_c2.OutVol * 6) * (sw35xx_c2.OutCur * 25 / 10) / 1000000);
  u8g2_DrawStr(&u8g2, 43, 56, buf);
  u8g2_DrawStr(&u8g2, 15, 22, "C1");
  u8g2_DrawStr(&u8g2, 15, 56, "C2");
  u8g2_SetFont(&u8g2, u8g2_font_siji_t_6x10);

  if (sw35xx_c1.state.pro_fastcharge)
    u8g2_DrawGlyph(&u8g2, 0, 12, 0XE040);
  if (sw35xx_c1.state.buck_open)
    u8g2_DrawGlyph(&u8g2, 0, 27, 0XE040 + 3);

  if (sw35xx_c2.state.pro_fastcharge)
    u8g2_DrawGlyph(&u8g2, 0, 46, 0XE040);
  if (sw35xx_c2.state.buck_open)
    u8g2_DrawGlyph(&u8g2, 0, 61, 0XE040 + 3);

  u8g2_SetFont(&u8g2, u8g2_font_helvB10_tr);
  sprintf(buf, "%d.%02dV", (sw35xx_c1.OutVol * 6) / 1000, ((sw35xx_c1.OutVol * 6) % 1000) / 10);
  u8g2_DrawStr(&u8g2, 88, 16, buf);
  sprintf(buf, "%d.%02dA", (sw35xx_c1.OutCur * 25 / 10) / 1000, ((sw35xx_c1.OutCur * 25 / 10) % 1000) / 10);
  u8g2_DrawStr(&u8g2, 88, 32 - 1, buf);
  sprintf(buf, "%d.%02dV", (sw35xx_c2.OutVol * 6) / 1000, ((sw35xx_c2.OutVol * 6) % 1000) / 10);
  u8g2_DrawStr(&u8g2, 88, 48 + 1, buf);
  sprintf(buf, "%d.%02dA", (sw35xx_c2.OutCur * 25 / 10) / 1000, ((sw35xx_c2.OutCur * 25 / 10) % 1000) / 10);
  u8g2_DrawStr(&u8g2, 88, 64, buf);

  u8g2_SendBuffer(&u8g2);
}

static void oledWeatherSurface(void)
{
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_wqy13_t_gb2312);
  u8g2_DrawUTF8(&u8g2, 40, 15, "气象站");
  u8g2_SendBuffer(&u8g2);
}

static void oledClockSurface(void){
  time_t now;
  struct tm timeinfo;
  char buf[64] = {0};

  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_wqy13_t_gb2312);
  u8g2_DrawUTF8(&u8g2, 40, 15, "时钟");
  u8g2_SetFont(&u8g2, u8g2_font_helvB10_tr);
  time(&now);
  localtime_r(&now, &timeinfo);
  sprintf(buf,"%02d:%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
  u8g2_DrawStr(&u8g2, 0, 32, buf);
  sprintf(buf,"%d/%d/%d",timeinfo.tm_year,timeinfo.tm_mon,timeinfo.tm_mday);
  u8g2_DrawStr(&u8g2, 0, 48, buf);
  strftime(buf,sizeof(timeinfo),"%c",&timeinfo);
  u8g2_DrawStr(&u8g2, 0, 64, buf);
  u8g2_SendBuffer(&u8g2);
}

static void oledSettingSurface(void)
{
  uint8_t current_selection = 0;
  uint8_t exit_flag = 0;
  for (;;)
  {
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_wqy13_t_gb2312);
    u8g2_SetFontRefHeightAll(&u8g2);
    current_selection = u8g2_UserInterfaceSelectionList(&u8g2, "设置", 1, "参数设置\n无线配置\n系统监控\n固件更新\n注册\n关于\n<-返回");
    ESP_LOGI(TAG, "oledtask!\n");
    switch (current_selection)
    {
    case 1:
      for (;;)
      {
        current_selection = u8g2_UserInterfaceSelectionList(&u8g2, "参数设置", 1, "C口功率限制\n快充协议限制\nRGB指示灯\n敲击控制\n<-返回");
        switch (current_selection)
        {
        case 1:
          oledPowerLimitTask();
          break;
        case 2:
          break;
        case 3:
          break;
        case 4:
          break;
        case 5:
          EXIT_MENU_SET
          break;
        }
        EXIT_MENU_CHECK
      }
      break;
    case 2:
      for (;;)
      {
        current_selection = u8g2_UserInterfaceSelectionList(&u8g2, "无线配置", 1, "Wifi状态\n配网\n远程控制\n蓝牙控制\n<-返回");
        switch (current_selection)
        {
        case 1:
          oledWifiShowTask();
          break;
        case 2:
          oledSmartConfigTask();
          break;
        case 3:
          break;
        case 4:
          break;
        case 5:
          EXIT_MENU_SET
          break;
        }
        EXIT_MENU_CHECK
      }
      break;
    case 3:
      for (;;)
      {
        current_selection = u8g2_UserInterfaceSelectionList(&u8g2, "系统监控", 1, "电压电流\n快充协议\n异常代码\n<-返回");
        switch (current_selection)
        {
        case 1:
          oledVIShowTask();
          break;
        case 2:
          oledProtocolShowTask();
          break;
        case 3:
          oledFaultShowTask();
          break;
        case 4:
          EXIT_MENU_SET
          break;
        }
        EXIT_MENU_CHECK
      }
      break;
    case 4:
      for (;;)
      {
        current_selection = u8g2_UserInterfaceSelectionList(&u8g2, "固件更新", 1, "手动更新\n自动更新\n<-返回");
        switch (current_selection)
        {
        case 1:
          oledManualOTATask();
          break;
        case 2:
          oledAutoOTATask();
          break;
        case 3:
          EXIT_MENU_SET
          break;
        }
        EXIT_MENU_CHECK
      }
      break;
    case 5:
      for (;;)
      {
        current_selection = u8g2_UserInterfaceSelectionList(&u8g2, "注册", 1, "自动注册\n注册码\n<-返回");
        switch (current_selection)
        {
        case 1:
          break;
        case 2:
          break;
        case 3:
          EXIT_MENU_SET
          break;
        }
        EXIT_MENU_CHECK
      }
      break;
    case 6:
      current_selection = u8g2_UserInterfaceSelectionList(&u8g2, "关于", 1, "版权所有\n伦敦烟云smog\n<-返回");
      break;
    case 7:
      EXIT_MENU_SET
      break;
    }
    EXIT_MENU_CHECK
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void oledTask(void *pvParameters)
{
  uint8_t Mode = 0; // 0-3分别对应主界面1，主界面2，主界面3，设置姐买你
  for (;;)
  {
    uint8_t event = u8x8_GetMenuEvent(u8g2_GetU8x8(&u8g2));
    switch (Mode)
    {
    case 0:
      oledChargeSurface();
      break;
    case 1:
      oledWeatherSurface();
      break;
    case 2:
      oledClockSurface();
      break;
    case 3:
      oledSettingSurface();
      Mode = 0;
      break;
    }
    if (event == U8X8_MSG_GPIO_MENU_SELECT)
    {
      break;
    }
    else if (event == U8X8_MSG_GPIO_MENU_NEXT || event == U8X8_MSG_GPIO_MENU_DOWN)
    {
      Mode = (Mode >= 3 ? 3 : Mode + 1);
    }
    else if (event == U8X8_MSG_GPIO_MENU_PREV || event == U8X8_MSG_GPIO_MENU_UP)
    {
      Mode = (Mode <= 0 ? 0 : Mode - 1);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  vTaskDelete(NULL);
}

void oledInitMessageTask(uint8_t num,char *state){
  char buf[25] = {0};
	u8g2_SetFont(&u8g2, u8g2_font_wqy13_t_gb2312);
  switch(num){
    case 1:
      sprintf(buf,"内部外设初始化...%s",state);
      u8g2_DrawUTF8(&u8g2, 0, 15, buf);    
      break;
    case 2:
      sprintf(buf,"功率芯片初始化...%s",state);
      u8g2_DrawUTF8(&u8g2, 0, 31, buf);
      break;
    case 3:
      sprintf(buf,"加速度计初始化...%s",state);
      u8g2_DrawUTF8(&u8g2, 0, 47, buf);
      break;
    case 4:
      sprintf(buf,"WIFI网络初始化...%s",state);
      u8g2_DrawUTF8(&u8g2, 0, 63, buf);
      break;
  }
  u8g2_SendBuffer(&u8g2);
}
