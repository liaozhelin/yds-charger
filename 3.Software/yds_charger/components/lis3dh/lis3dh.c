#include "lis3dh.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
// For VS Code linter
#include "../../build/config/sdkconfig.h"

#define _I2C_NUMBER(num) I2C_NUM_0
#define I2C_NUMBER(num) _I2C_NUMBER(num)

esp_err_t LIS3DH_I2C_Init(void)
{
#if CONFIG_LIS3DH_INIT_I2C_ENABLE
    int i2c_master_port = I2C_NUMBER(CONFIG_LIS3DH_I2C_PERIPH_NUM);
    i2c_config_t conf;
    memset(&conf, 0, sizeof(i2c_config_t));
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = CONFIG_LIS3DH_I2C_SDA_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = CONFIG_LIS3DH_I2C_SCL_PIN;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = CONFIG_LIS3DH_I2C_SPEED;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
#else
    return ESP_OK;
#endif
}

/**
 * @brief Test code to write esp-i2c-slave
 *        Master device write data to slave(both esp32),
 *        the data will be stored in slave buffer.
 *        We can read them out from slave buffer.
 *
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|
 *
 */
esp_err_t LIS3DH_WriteReg(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (CONFIG_LIS3DH_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, reg, 1);
    i2c_master_write_byte(cmd, value, 1);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUMBER(CONFIG_LIS3DH_I2C_PERIPH_NUM), cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
    {
        ESP_LOGE("LIS3DH", "ReadReg failed (err: 0x%X)", ret);
    }

    return ret;
}

/**
 * @brief test code to read esp-i2c-slave
 *        We need to fill the buffer of esp slave device, then master can read them out.
 *
 * _______________________________________________________________________________________
 * | start | slave_addr + rd_bit +ack | read n-1 bytes + ack | read 1 byte + nack | stop |
 * --------|--------------------------|----------------------|--------------------|------|
 *
 */
esp_err_t LIS3DH_ReadReg(uint8_t reg, uint8_t *dataRead)
{
    esp_err_t ret = ESP_FAIL;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (CONFIG_LIS3DH_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, reg, 1);
    // i2c_master_stop(cmd); //we need a repeated start

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (CONFIG_LIS3DH_I2C_ADDRESS << 1) | I2C_MASTER_READ, 1);
    i2c_master_read_byte(cmd, dataRead, 1);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUMBER(CONFIG_LIS3DH_I2C_PERIPH_NUM), cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
    {
        ESP_LOGE("LIS3DH", "ReadReg failed (err: 0x%X)", ret);
    }
    return ret;
}

esp_err_t LIS3DH_ConfigureMotionDetection(void)
{
    esp_err_t ret = ESP_OK;

    uint8_t ctrl1 = 0b00110111; // ODR-速率400Hz  LPen-关闭  x,y,z轴使能
    uint8_t ctrl2 = 0b00000000; // high pass settings
    uint8_t ctrl3 = 0b00000000; // enIrqPin1中断全关
    uint8_t ctrl4 = 0b10000000; // BDU阻塞更新 大端数据 量程2G 高分辨率模式关 自检关闭 SPI接口关闭
    uint8_t ctrl5 = 0b00000000; // 中断 FIFO全关
    // uint8_t ctrl6;           // not used
    // uint8_t int1cfg = 0b101010; //((axisEn & 0b001) << 1) & ((axisEn & 0b010) << 3) & ((axisEn & 0b100) << 5);
    // uint8_t int1ths = threshold & 0x7F;
    // uint8_t int1dur = duration & 0x7F;

    ret |= LIS3DH_WriteReg(LIS3DH_REG_CTRL1, ctrl1);
    vTaskDelay(pdMS_TO_TICKS(110));
    ret |= LIS3DH_WriteReg(LIS3DH_REG_CTRL2, ctrl2); // Enable high pass filter
    ret |= LIS3DH_WriteReg(LIS3DH_REG_CTRL3, ctrl3); // enable/disable IRQ pin for AOI1
    ret |= LIS3DH_WriteReg(LIS3DH_REG_CTRL4, ctrl4); // set sensor scale (2g, 4g, 8g, 16g)
    ret |= LIS3DH_WriteReg(LIS3DH_REG_CTRL5, ctrl5); // set interrupt latch and 4D/6D mode

    // uint8_t ref;
    // ret |= LIS3DH_ReadReg(LIS3DH_REG_REFERENCE, &ref);    // read the reference register to initialize high pass filter
    // ret |= LIS3DH_WriteReg(LIS3DH_REG_INT1_THS, int1ths); // threshold in LSB count (1LSB = scale/128)
    // ret |= LIS3DH_WriteReg(LIS3DH_REG_INT1_DUR, int1dur); // duration in 1/ODR unit

    // vTaskDelay(pdMS_TO_TICKS(20)); // delay to avoid first IRQ at boot

    // ret |= LIS3DH_WriteReg(LIS3DH_REG_INT1_CFG, int1cfg); // OR combination of axis high

    if (ret != ESP_OK)
    {
        ESP_LOGE("LIS3DH", "Motion Detect Config Failed (err: 0x%X)", ret);
    }
    return ret;
}

// static xQueueHandle   gpio_evt_queue = NULL;
// static void IRAM_ATTR gpio_isr_handler(void *arg)
// {
//     uint32_t gpio_num = (uint32_t)arg;
//     xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
// }

esp_err_t LIS3DH_Init()
{
    esp_err_t err = ESP_OK;
    // initialize I2C master driver
    err = LIS3DH_I2C_Init();
    if (err != ESP_OK)
    {
        ESP_LOGE("LIS3DH", "Failed to initialize I2C");
        return err;
    }
    // test communication by reading the WHO AM I register
    uint8_t dr = 0;
    err = LIS3DH_ReadReg(LIS3DH_REG_WHO_AM_I, &dr);
    if (err != ESP_OK)
        ESP_LOGE("LIS3DH", "ReadReg failed (%d)", err);

    if (dr != 0x33)
    {
        ESP_LOGE("LIS3DH", "Unkown LIS3DH ID (0x%02X, expected 0x33)", dr);
        return ESP_FAIL;
    }
    err = LIS3DH_ConfigureMotionDetection();
    //###############################################################
    // TEST IRQ SETTINGS

    // LIS3DH_ConfigureMotionDetection(2, 1, 0b111, 1, 1, 1, 0, 8, 1);
    // LIS3DH_ReadReg(LIS3DH_REG_STATUS_AUX, &dr);
    // ESP_LOGW("IRQ", "0x%02X", dr);
    //  while (retry--)
    //  {
    //      if (xQueueReceive(gpio_evt_queue, &io_num, pdMS_TO_TICKS(10)))
    //      {
    //          if (gpio_get_level(CONFIG_ELOPS_ACCELEROMETER_INT1_PIN))
    //          {
    //              ESP_LOGW("IRQ", "IRQ PIN HIGH");
    //          }

    //         LIS3DH_ReadReg(LIS3DH_REG_INT1_SRC, &dr);
    //         ESP_LOGW("IRQ", "0x%02X", dr);
    //     }
    // }
    // gpio_config_t io_conf;
    // io_conf.intr_type    = GPIO_PIN_INTR_POSEDGE;
    // io_conf.pin_bit_mask = 1 << CONFIG_ELOPS_ACCELEROMETER_INT1_PIN;
    // io_conf.mode         = GPIO_MODE_INPUT;
    // io_conf.pull_up_en   = 0;
    // io_conf.pull_down_en = 0;
    // gpio_config(&io_conf);

    // // install gpio isr service
    // gpio_install_isr_service(0);
    // // hook isr handler for specific gpio pin
    // gpio_isr_handler_add(CONFIG_ELOPS_ACCELEROMETER_INT1_PIN, gpio_isr_handler,
    //                      (void *)CONFIG_ELOPS_ACCELEROMETER_INT1_PIN);

    // gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // uint32_t io_num;
    // uint32_t retry = 3000;
    // while (retry--)
    // {
    //     if (xQueueReceive(gpio_evt_queue, &io_num, pdMS_TO_TICKS(10)))
    //     {
    //         if (gpio_get_level(CONFIG_ELOPS_ACCELEROMETER_INT1_PIN))
    //         {
    //             ESP_LOGW("IRQ", "IRQ PIN HIGH");
    //         }

    //         LIS3DH_ReadReg(LIS3DH_REG_INT1_SRC, &dr);
    //         ESP_LOGW("IRQ", "0x%02X", dr);
    //     }
    // }
    //###############################################################
    return err;
}

