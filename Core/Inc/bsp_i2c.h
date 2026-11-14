/**
  ******************************************************************************
  * @file    bsp_i2c.h
  * @brief   I2C1(PB6/PB7) 阻塞式收发、总线恢复和地址扫描。
  ******************************************************************************
  */
#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

extern I2C_HandleTypeDef hi2c1;

void bsp_i2c_init(void);

/* 总线被从机拉死时：DeInit -> GPIO 手动 9 个时钟 -> STOP -> Init */
void bsp_i2c_recover(void);

/* addr7 是 7 位地址，例如 0x3C、0x38 */
bool bsp_i2c_write(uint8_t addr7, const uint8_t *data, uint16_t len,
                   uint32_t timeout_ms);
bool bsp_i2c_read(uint8_t addr7, uint8_t *data, uint16_t len,
                  uint32_t timeout_ms);
bool bsp_i2c_probe(uint8_t addr7);

/* 返回发现的设备数量；found 回调可以为 NULL。 */
uint8_t bsp_i2c_scan(void (*found)(uint8_t addr7));

#endif /* BSP_I2C_H */
