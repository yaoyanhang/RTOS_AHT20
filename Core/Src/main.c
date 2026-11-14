/**
  ******************************************************************************
  * @file    main.c
  * @brief   任务一/任务二（3）：STM32F103C8T6 + uC/OS-III + AHT20
  *
  * 硬件：
  *   LED1  PC13（板载，低电平亮）
  *   LED2  PB12（外接，高电平亮）
  *   AHT20 PB6=SCL, PB7=SDA, 7 位地址 0x38
  *   USART1 PA9=TX, PA10=RX, 115200-8-N-1
  *
  * 时钟：HSE 8 MHz × 9 = 72 MHz；HSE 不起振时自动回退到 HSI/2 × 16 = 64 MHz。
  ******************************************************************************
  */
#include "main.h"
#include "app.h"
#include "board.h"
#include "bsp_i2c.h"
#include "bsp_uart.h"
#include "includes.h"

static void MX_GPIO_Init(void);

int main(void)
{
    OS_ERR err;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    bsp_uart_init();
    bsp_i2c_init();

    bsp_uart_puts("\r\nBOOT: STM32F103C8T6 / uC/OS-III / AHT20\r\n");
    bsp_uart_puts("I2C1: PB6=SCL, PB7=SDA, 100 kHz; AHT20 addr=0x38\r\n");

    App_Init();

    /* OSStartHighRdy() 最后会重新开中断；这里先关中断，避免启动瞬间
       SysTick 在第一个任务上下文建立之前进入 OS_CPU_SysTickHandler。 */
    __disable_irq();
    OSStart(&err);

    /* 正常情况下不会返回 */
    (void)err;
    while (DEF_TRUE)
    {
    }
}

/**
  * @brief 系统时钟：优先 72 MHz(HSE)，失败则 64 MHz(HSI)。
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    /* 先尝试外部 8 MHz 晶振，PLL ×9 = 72 MHz */
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL = RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        /* 没有晶振或晶振不起振：退回内部 HSI/2 ×16 = 64 MHz */
        HAL_RCC_DeInit();

        osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
        osc.HSIState = RCC_HSI_ON;
        osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
        osc.PLL.PLLState = RCC_PLL_ON;
        osc.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
        osc.PLL.PLLMUL = RCC_PLL_MUL16;

        if (HAL_RCC_OscConfig(&osc) != HAL_OK)
        {
            Error_Handler();
        }
    }

    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;   /* I2C1 在 APB1：36 MHz(或 32 MHz) */
    clk.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* LED1：PC13，低电平点亮，先写高再配置为输出，避免上电闪一下 */
    HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET);
    gpio.Pin = LED1_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED1_PORT, &gpio);

    /* LED2：PB12，高电平点亮 */
    HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_RESET);
    gpio.Pin = LED2_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED2_PORT, &gpio);
}

/**
  * @brief 致命错误：关中断，PC13 快闪。
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        LED1_TOGGLE();
        for (volatile uint32_t i = 0; i < 250000U; ++i)
        {
            __NOP();
        }
    }
}
