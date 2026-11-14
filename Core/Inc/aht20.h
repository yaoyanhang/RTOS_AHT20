/**
  ******************************************************************************
  * @file    aht20.h
  * @brief   AHT20 温湿度传感器驱动（I2C 地址 0x38）。
  ******************************************************************************
  */
#ifndef AHT20_H
#define AHT20_H

#include <stdbool.h>
#include <stdint.h>

/* AHT20 固定 7 位 I2C 地址：0x38（写 0x70，读 0x71） */
#define AHT20_I2C_ADDR          0x38U

typedef struct
{
    float   temperature;  /* 摄氏度 */
    float   humidity;     /* 相对湿度 %RH */
    uint8_t crc_ok;       /* 1: CRC 校验通过 */
    uint8_t valid;        /* 1: 本次数据有效 */
} aht20_data_t;

/* 上电 >=40 ms 后调用；会自动判断是否需要发送 0xBE 校准命令。 */
bool aht20_init(void);

/* 触发一次测量（内部等待 80 ms），成功返回 true。 */
bool aht20_read(aht20_data_t *out);

/* 最近一次读到的状态字节，调试用。 */
uint8_t aht20_last_status(void);

/* 毫秒延时钩子：驱动内部不直接调用 HAL_Delay。
   RTOS 工程在 app.c 中提供强定义（OSTimeDlyHMSM），
   裸机测试时使用 aht20.c 中的 __weak 版本（HAL_Delay）。 */
void aht20_delay_ms(uint32_t ms);

#endif /* AHT20_H */
