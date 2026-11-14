/**
  ******************************************************************************
  * @file    stm32f1xx_hal_msp.c
  * @brief   HAL 底层引脚/时钟初始化。
  ******************************************************************************
  */
#include "main.h"
#include "bsp_i2c.h"
#include "bsp_uart.h"

void HAL_MspInit(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* 保留 SWD(PA13/PA14)，关闭 JTAG，释放 PB3/PB4/PA15 */
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef gpio = {0};

    if (hi2c->Instance == I2C1)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C1_CLK_ENABLE();

        /* PB6=SCL, PB7=SDA，复用开漏，外部/模块上拉 */
        gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        gpio.Mode = GPIO_MODE_AF_OD;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &gpio);
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
        __HAL_RCC_I2C1_CLK_DISABLE();
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio = {0};

    if (huart->Instance == USART1)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_USART1_CLK_ENABLE();

        /* PA9=TX 复用推挽，PA10=RX 上拉输入 */
        gpio.Pin = GPIO_PIN_9;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &gpio);

        gpio.Pin = GPIO_PIN_10;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOA, &gpio);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
        __HAL_RCC_USART1_CLK_DISABLE();
    }
}
