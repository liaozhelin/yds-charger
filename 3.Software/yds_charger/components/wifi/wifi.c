/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-02-10 16:05:01
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-06-13 20:45:45
 * @Description: wifi_bsp
 */
/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "wifi.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_smartconfig.h"
#include "lwip/err.h"
#include "lwip/sys.h"

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define ESPTOUCH_DONE_BIT BIT2

static const char *TAG = "wifi station";

static int s_retry_num = 0;

static void smartconfig_example_task(void *parm);

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    //当Wifi事件组的sta开始事件发生后,连接wifi
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    //当wifi事件组还未连接到wifi时,尝试连接WIFI事件
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // 当Wifi连接尝试次数小于设定的最大尝试次数时候
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect(); //尝试连接
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }
        else
        {
            // xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL); //开始配网
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT); //超出最大次数,连接失败
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    //获取IP事件
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        //打印出对应的IP
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        // 连接到了WIFI,获取IP后,给事件组设置对应标志位
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    // Smart-config的处理函数
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
    {
        ESP_LOGI(TAG, "Scan done");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
    {
        ESP_LOGI(TAG, "Found channel");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        nvs_handle_t wificonfig_handle;
        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = {0};
        uint8_t password[65] = {0};
        uint8_t rvd_data[33] = {0};
        ESP_LOGI(TAG, "Got SSID and password");
        ESP_LOGI(TAG, "Got SSID and password");
        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true)
        {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2)
        {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i = 0; i < 33; i++)
            {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK(nvs_open("WifiConfigFlag",NVS_READWRITE,&wificonfig_handle));
        ESP_ERROR_CHECK(nvs_set_u8(wificonfig_handle,"WifiConfigFlag",wifi_configed));
        ESP_ERROR_CHECK(nvs_commit(wificonfig_handle));
        nvs_close(wificonfig_handle);

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
    {
        // Smart-config全部完成
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

/* wifi初始化函数 mode[0:1] 0:初次初始化 1:手动配网 */
uint8_t wifi_init_sta(uint8_t mode)
{
    uint8_t err = 0;
    esp_netif_t *sta_netif;
    EventBits_t bits;
    /* 创建事件组 */
    if (!mode)
    {
        s_wifi_event_group = xEventGroupCreate();
        /* 初始化netif以及创建默认的事件组循环（默认事件组对用户隐藏，只能通过API控制） */
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        /* 通过API 将Wifi配置创建esp_netif对象,注册默认的Wifi处理程序 */
        sta_netif = esp_netif_create_default_wifi_sta();
        assert(sta_netif);
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        /* 将事件处理函数注册到默认的事件组循环。这里主要是两个事件，一个是连接WIFI,一个是获取IP */ //原:esp_event_handler_instance_register
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

        // wifi_config_t wifi_config = {
        //     .sta = {
        //         .ssid = EXAMPLE_ESP_WIFI_SSID,
        //         .password = EXAMPLE_ESP_WIFI_PASS,
        //         .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        //         .pmf_cfg = {
        //             .capable = true,
        //             .required = false},
        //     },
        // };
        /* 设定wifi的模式,启动wifi */
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        //ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(TAG, "wifi_init_sta finished.");

        /* 设定连接Wifi的事件组等待。标志位为WIFI_CONNECTED_BIT（Wifi已连接）和WIFI_FAIL_BIT（Wifi未连接），有一个发生则退出，否则阻塞在这里 */
        bits = xEventGroupWaitBits(s_wifi_event_group,
                                   WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                   pdFALSE,
                                   pdFALSE,
                                   portMAX_DELAY);

        /* 解除阻塞状态后，通过返回的bits位判断Wifi事件组的状态，并打印相关信息 */
        if (bits & WIFI_CONNECTED_BIT)
        {
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                     EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
            esp_wifi_set_ps(WIFI_PS_NONE); //设置wifi为不省电模式
            err = 1;
        }
        else if (bits & WIFI_FAIL_BIT)
        {
            ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                     EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
            err = 0;
        }
        else
        {
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
            err = 0;
        }
        /* 取消事件组的注册，删除事件组 */
        ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
        ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
        vEventGroupDelete(s_wifi_event_group);
    }
    else
    {
        s_wifi_event_group = xEventGroupCreate();

        esp_netif_init();
        esp_event_loop_create_default();
        sta_netif = esp_netif_create_default_wifi_sta();
        //assert(sta_netif);
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        esp_wifi_init(&cfg);
        
        //esp_wifi_disconnect();
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);
        esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_start();
        xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL); //开始配网
        bits = xEventGroupWaitBits(s_wifi_event_group,
                                   ESPTOUCH_DONE_BIT,
                                   pdFALSE,
                                   pdFALSE,
                                   portMAX_DELAY);

        ESP_LOGI(TAG, "CONFIG SUCCESS!!");
        err = 1;
        ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
        ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
        ESP_ERROR_CHECK(esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler));
        vEventGroupDelete(s_wifi_event_group);
   }
   return err;
}

static void smartconfig_example_task(void *parm)
{
    EventBits_t uxBits;
    // 设定配网方式为ESP_TOUCH
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    // 开启智能配网
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    while (1)
    {
        // 阻塞,等待智能配网完成
        uxBits = xEventGroupWaitBits(s_wifi_event_group, ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        // 直接连接,无配网
        // if (uxBits & WIFI_CONNECTED_BIT)
        // {
        //     ESP_LOGI(TAG, "WiFi Connected to ap");
        // }
        // 智能配网完成,打印信息,关闭智能配网,然后删除这个task
        if (uxBits & ESPTOUCH_DONE_BIT)
        {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}
