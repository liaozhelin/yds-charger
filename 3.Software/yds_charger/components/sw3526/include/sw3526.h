/*
 * @Author: [LiaoZhelin]
 * @Date: 2022-04-07 22:32:34
 * @LastEditors: [LiaoZhelin]
 * @LastEditTime: 2022-05-02 16:32:41
 * @Description: 
 */
#ifndef _SW3526_H_
#define _SW3526_H_

#include "software_i2c.h"

#define SW35XX_ADDRESS 0x78

#define SW35XX_IC_VERSION 0x01
#define SW35XX_BUCK_DATE_H 0X03
#define SW35XX_BUCK_DATE_L 0X04
#define SW35XX_BUCK_LIMIT_I 0X05
#define SW35XX_FCX_STATUS 0x06
#define SW35XX_PWR_STATUS 0x07
#define SW35XX_PWR_FAULTS 0X0B
#define SW35XX_I2C_WRITE_ENABLE 0x12
#define SW35XX_BUCK_SHUTDOWN 0x13

#define SW35XX_ADC_VIN_H 0x30
#define SW35XX_ADC_VOUT_H 0x31

#define SW35XX_ADC_IOUT_H 0x33

#define SW35XX_ADC_DATA_TYPE 0x3a
#define SW35XX_ADC_DATA_BUF_H 0x3b
#define SW35XX_ADC_DATA_BUF_L 0x3c

#define SW35XX_POW_STATUS 0x68
#define SW35XX_PWR_CONF 0xa7

#define SW35XX_PD_SRC_REQ 0x70
#define SW35XX_FAST_CHARGE_CON0 0xa8
#define SW35XX_FAST_CHARGE_CON1 0xa9
#define SW35XX_FAST_CHARGE_CON2 0xaa
#define SW35XX_FAST_CHARGE_CON3 0xab
#define SW35XX_FAST_CHARGE_CON4 0xac

#define SW35XX_VID_CONF0 0xae
#define SW35XX_VID_CONF1 0xaf

#define I2C_RETRIES 10

typedef enum{
    NONE_PROC = 0,
    QC2_0,
    QC3_0,
    FCP,
    SCP,
    PD_FIX,
    PD_PPS,
    PE1_1,
    PE2_0,
    VOOC,
    SFCP,
    AFC
}chargeprotocol_t;

typedef enum{
    NONE_PD = 0,
    PD2_0,
    PD3_0
}pdversion_t;

typedef struct{
    uint16_t OutVol;// 输出电压
    uint16_t OutCur;// 输出电流
    uint16_t InVol;// 输入电压
    struct{
        bool pro_fastcharge;// 0-非快充协议  1-快充协议 
        bool val_fastcharge;// 0-电压处于非快充  1-电压处于快充
        bool out_open;// 0-端口关闭  1-端口打开
        bool buck_open;// 0-buck关闭  1-buck打开
        bool val_input_upmax;// 0-输入正常  1-输入过压
        bool tem_alarm_upmax;// 0-温度正常  1-温度报警
        bool tem_shutdown;// 0-温度正常  1-高温关机保护
        bool short_citcuit;// 0-输出正常  1-输出短路
    }state;
    chargeprotocol_t protocol;//当前协议
    pdversion_t pdversion;//PD版本
}sw35xx_t;

sw35xx_t sw35xx_c1,sw35xx_c2;

void SW35xxInit(void);
uint8_t SW35xxReadReg(uint8_t reg,swi2c_t port);
uint8_t SW35xxWriteReg(uint8_t reg,uint8_t data,swi2c_t port);
void SW35XXUpdate(void);

#endif