/* Advanced HTTPS OTA example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include <driver/gpio.h>

#if CONFIG_BOOTLOADER_APP_ANTI_ROLLBACK
#include "esp_efuse.h"
#endif

#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif

#if CONFIG_BT_BLE_ENABLED || CONFIG_BT_NIMBLE_ENABLED
#include "ble_api.h"
#endif

static const char *TAG = "advanced_https_ota_example";
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

#define OTA_URL_SIZE 256

static esp_err_t validate_image_header(esp_app_desc_t *new_app_info)
{
    if (new_app_info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_partition_t *running = esp_ota_get_running_partition(); //获取当前运行的应用程序的分区信息
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) { //返回正在运行引用程序分区的版本号
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);  //打印版本号
    }

#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECK
    if (memcmp(new_app_info->version, running_app_info.version, sizeof(new_app_info->version)) == 0) { //比较版本号，如果一样则退出
        ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
        return ESP_FAIL;
    }
#endif

#ifdef CONFIG_BOOTLOADER_APP_ANTI_ROLLBACK
    /**
     * Secure version check from firmware image header prevents subsequent download and flash write of
     * entire firmware image. However this is optional because it is also taken care in API
     * esp_https_ota_finish at the end of OTA update procedure.
     */
    const uint32_t hw_sec_version = esp_efuse_read_secure_version();//从efuse字段返回secure_version
    if (new_app_info->secure_version < hw_sec_version) { //判断硬件版本号，如果比当前版本更低则不升级
        ESP_LOGW(TAG, "New firmware security version is less than eFuse programmed, %d < %d", new_app_info->secure_version, hw_sec_version);
        return ESP_FAIL;
    }
#endif

    return ESP_OK; //都满足，返回success
}

static esp_err_t _http_client_init_cb(esp_http_client_handle_t http_client)
{
    esp_err_t err = ESP_OK;
    /* Uncomment to add custom headers to HTTP request */
    // err = esp_http_client_set_header(http_client, "Custom-Header", "Value");
    return err;
}

void advanced_ota_example_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Starting Advanced OTA example");

    esp_err_t ota_finish_err = ESP_OK;
    esp_http_client_config_t config = {
        .url = CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL, //服务URL
        .cert_pem = (char *)server_cert_pem_start, //SSL证书
        .timeout_ms = CONFIG_EXAMPLE_OTA_RECV_TIMEOUT,
        .keep_alive_enable = true,
    };

#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL_FROM_STDIN  //从文件系统中读取URL
    char url_buf[OTA_URL_SIZE];
    if (strcmp(config.url, "FROM_STDIN") == 0) {
        example_configure_stdin_stdout();
        fgets(url_buf, OTA_URL_SIZE, stdin);
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0';
        config.url = url_buf;
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong firmware upgrade image url");
        abort();
    }
#endif

#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
        .http_client_init_cb = _http_client_init_cb, // Register a callback to be invoked after esp_http_client is initialized
#ifdef CONFIG_EXAMPLE_ENABLE_PARTIAL_HTTP_DOWNLOAD
        .partial_http_download = true,
        .max_http_request_size = CONFIG_EXAMPLE_HTTP_REQUEST_SIZE,
#endif
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle); //调用begin，开始ota，检查http地址有效性，返回https_ota_handle
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
        vTaskDelete(NULL);
    }

    esp_app_desc_t app_desc;
    err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);//从镜像描述中，读取固件版本等信息信息
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");
        goto ota_end;
    }
    err = validate_image_header(&app_desc);//校验镜像头，判断是否需要更新
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "image header verification failed");
        goto ota_end;
    }

    while (1) {
        err = esp_https_ota_perform(https_ota_handle); //从HTTP流中读取图像数据并将其写入OTA分区
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        // esp_https_ota_perform returns after every read operation which gives user the ability to
        // monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of image
        // data read so far.
        ESP_LOGD(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
    }

    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {  //检测是否接收到完整数据
        // the OTA image was not completely received and user can customise the response to this situation.
        ESP_LOGE(TAG, "Complete data was not received.");  //没有升级成功
    } else {
        ota_finish_err = esp_https_ota_finish(https_ota_handle);  //清理HTTPS OTA固件升级和关闭HTTPS连接，并切换启动分区到新镜像OTA分区
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
            ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            esp_restart(); //当esp_https_ota_finish后，调用此从新镜像启动
        } else {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                ESP_LOGE(TAG, "Image validation failed, image is corrupted");
            }
            ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);
            vTaskDelete(NULL);
        }
    }

ota_end:
    esp_https_ota_abort(https_ota_handle);//清理HTTPS OTA固件升级和关闭HTTPS连接，停止OTA更新
    ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed");
    vTaskDelete(NULL);//删除OTA任务
}

void OTA_Init(void){
    ESP_ERROR_CHECK(esp_netif_init()); //初始化底层的TCP/IP协议栈
    ESP_ERROR_CHECK(esp_event_loop_create_default()); //创建默认的事件循环
    
    ESP_ERROR_CHECK(example_connect());//连接Wifi

    esp_wifi_set_ps(WIFI_PS_NONE); //设置wifi为不省电模式
}
// void app_main(void)
// {
//     // Initialize NVS.
//     esp_err_t err = nvs_flash_init();
//     if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
//         // partition table. This size mismatch may cause NVS initialization to fail.
//         // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
//         // If this happens, we erase NVS partition and initialize NVS again.
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         err = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK( err );

//     ESP_ERROR_CHECK(esp_netif_init()); //初始化底层的TCP/IP协议栈
//     ESP_ERROR_CHECK(esp_event_loop_create_default()); //创建默认的事件循环

//     /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
//      * Read "Establishing Wi-Fi or Ethernet Connection" section in
//      * examples/protocols/README.md for more information about this function.
//     */
//     ESP_ERROR_CHECK(example_connect());//连接Wifi

// #if defined(CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE)
//     /**
//      * We are treating successful WiFi connection as a checkpoint to cancel rollback  将成功Wifi连接作为一个检查点
//      * process and mark newly updated firmware image as active. For production cases,
//      * please tune the checkpoint behavior per end application requirement.
//      */
//     const esp_partition_t *running = esp_ota_get_running_partition();//获取当前运行版本号
//     esp_ota_img_states_t ota_state;
//     if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) { //返回运行分区的状态
//         if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) { //第一次从此分区启动
//             if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) { //判断当前运行的APP状态是否工作正常
//                 ESP_LOGI(TAG, "App is valid, rollback cancelled successfully");
//             } else {
//                 ESP_LOGE(TAG, "Failed to cancel rollback");
//             }
//         }
//     }
// #endif

// #if CONFIG_EXAMPLE_CONNECT_WIFI
// #if !CONFIG_BT_ENABLED
//     /* Ensure to disable any WiFi power save mode, this allows best throughput
//      * and hence timings for overall OTA operation.
//      */
//     esp_wifi_set_ps(WIFI_PS_NONE); //设置wifi为不省电模式
// #else //有蓝牙的状态
//     /* WIFI_PS_MIN_MODEM is the default mode for WiFi Power saving. When both
//      * WiFi and Bluetooth are running, WiFI modem has to go down, hence we
//      * need WIFI_PS_MIN_MODEM. And as WiFi modem goes down, OTA download time
//      * increases.
//      */
//     esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
// #endif // CONFIG_BT_ENABLED
// #endif // CONFIG_EXAMPLE_CONNECT_WIFI

// #if CONFIG_BT_BLE_ENABLED || CONFIG_BT_NIMBLE_ENABLED
//     esp_ble_helper_init(); //通过蓝牙OTA??
// #endif

//     xTaskCreate(&advanced_ota_example_task, "advanced_ota_example_task", 1024 * 8, NULL, 5, NULL);
//     gpio_pad_select_gpio(3);
//     gpio_set_direction(3, GPIO_MODE_OUTPUT);
//     for(;;){
//         gpio_set_level(3, 1);
//         vTaskDelay(pdMS_TO_TICKS(200));
//         gpio_set_level(3, 0);
//         vTaskDelay(pdMS_TO_TICKS(200));
//     }
// }
