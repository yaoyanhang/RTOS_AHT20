/**
  ******************************************************************************
  * @file    bsp_uart.h
  * @brief   USART1(PA9/PA10) 115200-8-N-1，轻量日志输出（不依赖 printf）。
  ******************************************************************************
  */
#ifndef BSP_UART_H
#define BSP_UART_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

extern UART_HandleTypeDef huart1;

void bsp_uart_init(void);
void bsp_uart_putc(char c);
void bsp_uart_puts(const char *s);
void bsp_uart_put_u32(uint32_t value);
void bsp_uart_put_i32(int32_t value);
void bsp_uart_put_hex8(uint8_t value);
void bsp_uart_newline(void);

#endif /* BSP_UART_H */
