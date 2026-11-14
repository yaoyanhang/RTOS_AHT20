/**
  ******************************************************************************
  * @file    aht20.c
  * @brief   AHT20 温湿度传感器驱动。
  *
  * 关键时序（数据手册）：
  *   上电 40 ms -> 读状态，若 bit3=0 发 0xBE 0x08 0x00 -> 等 10 ms
  *   触发测量 0xAC 0x33 0x00 -> 等 80 ms -> 读 7 字节
  *   data[0].bit7 = busy；湿度 20 bit；温度 20 bit；data[6] = CRC8
  *
  * 温度：T = raw / 2^20 * 200 - 50
  * 湿度：RH = raw / 2^20 * 100
  ******************************************************************************
  */
#include "aht20.h"
#include "bsp_i2c.h"
#include "stm32f1xx_hal.h"

static uint8_t s_last_status;

/* 弱定义：裸机工程直接使用 HAL_Delay；RTOS 工程在 app.c 中提供强定义。 */
__weak void aht20_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

static uint8_t crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFFU;

    for (uint8_t i = 0; i < len; ++i)
    {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8U; ++bit)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (uint8_t)((crc << 1) ^ 0x31U);
            }
            else
            {
                crc = (uint8_t)(crc << 1);
            }
        }
    }

    return crc;
}

bool aht20_init(void)
{
    uint8_t status = 0;

    aht20_delay_ms(40);

    if (!bsp_i2c_read(AHT20_I2C_ADDR, &status, 1, 50))
    {
        s_last_status = 0xFFU;
        return false;
    }

    s_last_status = status;

    if ((status & 0x08U) == 0U)
    {
        uint8_t cmd[3] = {0xBEU, 0x08U, 0x00U};
        if (!bsp_i2c_write(AHT20_I2C_ADDR, cmd, sizeof(cmd), 50))
        {
            return false;
        }
        aht20_delay_ms(10);
    }

    return true;
}

bool aht20_read(aht20_data_t *out)
{
    uint8_t cmd[3] = {0xACU, 0x33U, 0x00U};
    uint8_t data[7] = {0};
    bool got = false;

    if (out == NULL)
    {
        return false;
    }

    out->valid = 0;
    out->crc_ok = 0;

    if (!bsp_i2c_write(AHT20_I2C_ADDR, cmd, sizeof(cmd), 50))
    {
        return false;
    }

    /* AHT20 每次转换约 80 ms；忙时只重复读状态，不重复触发转换 */
    for (uint8_t attempt = 0; attempt < 3U; ++attempt)
    {
        aht20_delay_ms(80);

        if (!bsp_i2c_read(AHT20_I2C_ADDR, data, sizeof(data), 100))
        {
            return false;
        }

        s_last_status = data[0];

        if ((data[0] & 0x80U) == 0U)
        {
            got = true;
            break;
        }

        aht20_delay_ms(20);
    }

    if (!got)
    {
        return false;
    }

    uint32_t humidity_raw = ((uint32_t)data[1] << 12) |
                            ((uint32_t)data[2] << 4) |
                            ((uint32_t)data[3] >> 4);

    uint32_t temperature_raw = ((uint32_t)(data[3] & 0x0FU) << 16) |
                               ((uint32_t)data[4] << 8) |
                               (uint32_t)data[5];

    out->humidity = ((float)humidity_raw * 100.0f) / 1048576.0f;
    out->temperature = (((float)temperature_raw * 200.0f) / 1048576.0f) - 50.0f;
    out->crc_ok = (crc8(data, 6) == data[6]) ? 1U : 0U;
    out->valid = 1U;

    return true;
}

uint8_t aht20_last_status(void)
{
    return s_last_status;
}
