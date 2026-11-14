/**
  ******************************************************************************
  * @file    bsp_uart.c
  * @brief   USART1 轻量日志输出，不依赖 printf / MicroLIB。
  ******************************************************************************
  */
#include "bsp_uart.h"
#include <stdbool.h>
#include <string.h>

UART_HandleTypeDef huart1;

static bool s_uart_ready;

void bsp_uart_init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200U;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    s_uart_ready = (HAL_UART_Init(&huart1) == HAL_OK);
}

void bsp_uart_putc(char c)
{
    if (s_uart_ready)
    {
        (void)HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, 20);
    }
}

void bsp_uart_puts(const char *s)
{
    if (s_uart_ready && (s != NULL))
    {
        (void)HAL_UART_Transmit(&huart1, (uint8_t *)s,
                                (uint16_t)strlen(s), 100);
    }
}

void bsp_uart_put_u32(uint32_t value)
{
    char buf[11];
    uint8_t i = 0;

    if (value == 0U)
    {
        bsp_uart_putc('0');
        return;
    }

    while ((value > 0U) && (i < sizeof(buf)))
    {
        buf[i++] = (char)('0' + (value % 10U));
        value /= 10U;
    }

    while (i > 0U)
    {
        bsp_uart_putc(buf[--i]);
    }
}

void bsp_uart_put_i32(int32_t value)
{
    if (value < 0)
    {
        bsp_uart_putc('-');
        bsp_uart_put_u32((uint32_t)(-value));
    }
    else
    {
        bsp_uart_put_u32((uint32_t)value);
    }
}

void bsp_uart_put_hex8(uint8_t value)
{
    static const char hex[] = "0123456789ABCDEF";
    bsp_uart_puts("0x");
    bsp_uart_putc(hex[(value >> 4) & 0x0FU]);
    bsp_uart_putc(hex[value & 0x0FU]);
}

void bsp_uart_newline(void)
{
    bsp_uart_puts("\r\n");
}
