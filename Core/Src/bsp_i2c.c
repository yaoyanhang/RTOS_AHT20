/**
  ******************************************************************************
  * @file    bsp_i2c.c
  * @brief   I2C1 初始化、阻塞收发、总线恢复。
  *
  * 总线恢复原理：如果从机在传输中途掉电/复位，可能一直把 SDA 拉低。
  * 此时主机先 DeInit，再把 SCL/SDA 当普通开漏 GPIO，手动发 9 个 SCL
  * 时钟，让从机把剩余位发完，最后补一个 STOP，再重新 Init。
  ******************************************************************************
  */
#include "bsp_i2c.h"
#include "main.h"
#include <stddef.h>

I2C_HandleTypeDef hi2c1;

static uint8_t s_error_count;

static void i2c_bus_clear(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
    HAL_Delay(1);

    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_RESET)
    {
        for (uint8_t i = 0; i < 9U; ++i)
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
            HAL_Delay(1);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
            HAL_Delay(1);
        }

        /* 手动 STOP：SCL 高时 SDA 由低变高 */
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
        HAL_Delay(1);
    }
}

static void i2c_hw_init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000U;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}

void bsp_i2c_init(void)
{
    i2c_bus_clear();
    i2c_hw_init();
}

void bsp_i2c_recover(void)
{
    (void)HAL_I2C_DeInit(&hi2c1);
    i2c_bus_clear();
    i2c_hw_init();
    s_error_count = 0;
}

static void i2c_note_error(void)
{
    s_error_count++;
    if (s_error_count >= 3U)
    {
        bsp_i2c_recover();
    }
}

bool bsp_i2c_write(uint8_t addr7, const uint8_t *data, uint16_t len,
                   uint32_t timeout_ms)
{
    if ((data == NULL) || (len == 0U))
    {
        return true;
    }

    if (HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(addr7 << 1),
                                (uint8_t *)data, len, timeout_ms) != HAL_OK)
    {
        i2c_note_error();
        return false;
    }

    s_error_count = 0;
    return true;
}

bool bsp_i2c_read(uint8_t addr7, uint8_t *data, uint16_t len,
                  uint32_t timeout_ms)
{
    if ((data == NULL) || (len == 0U))
    {
        return true;
    }

    if (HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(addr7 << 1),
                               data, len, timeout_ms) != HAL_OK)
    {
        i2c_note_error();
        return false;
    }

    s_error_count = 0;
    return true;
}

bool bsp_i2c_probe(uint8_t addr7)
{
    return HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr7 << 1), 2, 20) == HAL_OK;
}

uint8_t bsp_i2c_scan(void (*found)(uint8_t addr7))
{
    uint8_t count = 0;

    for (uint8_t addr = 0x08U; addr <= 0x77U; ++addr)
    {
        if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 1, 5) == HAL_OK)
        {
            count++;
            if (found != NULL)
            {
                found(addr);
            }
        }
    }

    return count;
}
