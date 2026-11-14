/**
  ******************************************************************************
  * @file    app.c
  * @brief   uC/OS-III 三个任务：
  *            Task1：PC13 LED，1 s 周期（亮 0.5 s / 灭 0.5 s）
  *            Task2：PB12 LED，3 s 周期（亮 1.5 s / 灭 1.5 s）
  *            Task3：AHT20 每 2 s 采集一次，串口输出问候语和温湿度
  *
  * 任务优先级：Task1 > Task2 > Task3，均为可抢占、按优先级调度。
  * 所有周期使用 OSTimeDlyHMSM(..., OS_OPT_TIME_PERIODIC, ...)，
  * 以“上一次唤醒时刻”为基准，避免任务执行时间造成周期累积漂移。
  ******************************************************************************
  */
#include "app.h"
#include "aht20.h"
#include "board.h"
#include "bsp_i2c.h"
#include "bsp_uart.h"
#include <stdbool.h>
#include <stdint.h>

/* ---------------- 任务栈（静态分配，避免使用堆） ---------------- */
static CPU_STK AppTaskStartStk[APP_TASK_START_STK_SIZE];
static CPU_STK AppTaskLed1Stk[APP_TASK_LED1_STK_SIZE];
static CPU_STK AppTaskLed2Stk[APP_TASK_LED2_STK_SIZE];
static CPU_STK AppTaskAht20Stk[APP_TASK_AHT20_STK_SIZE];

/* ---------------- 任务控制块 ---------------- */
OS_TCB AppTaskStartTCB;
OS_TCB AppTaskLed1TCB;
OS_TCB AppTaskLed2TCB;
OS_TCB AppTaskAht20TCB;

static void AppTaskLed1(void *p_arg);
static void AppTaskLed2(void *p_arg);
static void AppTaskAht20(void *p_arg);
static void AppTaskCreate(void);

/* 串口输出一位小数，不依赖 printf，节省 Flash 和栈 */
static void uart_put_1dec(float value)
{
    int32_t x;

    if (value >= 0.0f)
    {
        x = (int32_t)(value * 10.0f + 0.5f);
    }
    else
    {
        x = (int32_t)(value * 10.0f - 0.5f);
    }

    if (x < 0)
    {
        bsp_uart_putc('-');
        x = -x;
    }

    bsp_uart_put_u32((uint32_t)(x / 10));
    bsp_uart_putc('.');
    bsp_uart_putc((char)('0' + (x % 10)));
}

/**
  * @brief  AHT20 驱动的毫秒延时钩子。
  *
  * AHT20 初始化/转换需要 40 ms、80 ms 等等待。RTOS 运行后必须让出 CPU，
  * 不能使用 HAL_Delay() 死等；OS 未启动时（例如裸机测试）仍退回 HAL_Delay()。
  */
void aht20_delay_ms(uint32_t ms)
{
    OS_ERR err;

    if (OSRunning == OS_STATE_OS_RUNNING)
    {
        OSTimeDlyHMSM(0u, 0u, 0u, ms,
                      (OS_OPT)(OS_OPT_TIME_HMSM_STRICT), &err);
    }
    else
    {
        HAL_Delay(ms);
    }
}

/**
  * @brief  系统节拍初始化：HCLK / OS_CFG_TICK_RATE_HZ = 72 000 000 / 1000 = 72000。
  *
  * 必须在 OSStart() 之后、任务开始使用 OSTimeDly() 之前调用。
  */
static void BSP_Init(void)
{
    OS_CPU_SysTickInit(SystemCoreClock / OSCfg_TickRate_Hz);
}

/**
  * @brief  创建起始任务。真正的应用任务在起始任务里创建，
  *         这样 OSStart() 之前只需要一个任务控制块。
  */
void App_Init(void)
{
    OS_ERR err;

    OSInit(&err);
    if (err != OS_ERR_NONE)
    {
        Error_Handler();
    }

    OSTaskCreate(&AppTaskStartTCB,
                 "Start Task",
                 AppTaskStart,
                 (void *)0,
                 APP_TASK_START_PRIO,
                 &AppTaskStartStk[0],
                 APP_TASK_START_STK_SIZE / 10u,
                 APP_TASK_START_STK_SIZE,
                 0u,
                 0u,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);
    if (err != OS_ERR_NONE)
    {
        Error_Handler();
    }
}

/**
  * @brief  起始任务：初始化节拍、CPU，创建三个应用任务，然后挂起自己。
  */
void AppTaskStart(void *p_arg)
{
    OS_ERR err;

    (void)p_arg;

    BSP_Init();
    CPU_Init();

    bsp_uart_puts("\r\n============================================\r\n");
    bsp_uart_puts("  uC/OS-III + STM32F103C8T6 + AHT20\r\n");
    bsp_uart_puts("  SYSCLK = ");
    bsp_uart_put_u32(SystemCoreClock);
    bsp_uart_puts(" Hz, OS tick = 1000 Hz\r\n");
    bsp_uart_puts("  Task1: PC13 LED 1 s, Task2: PB12 LED 3 s\r\n");
    bsp_uart_puts("  Task3: AHT20 2 s, USART1 115200-8-N-1\r\n");
    bsp_uart_puts("============================================\r\n");

    AppTaskCreate();

    /* 起始任务完成使命，挂起；它的栈仍然保留，便于调试时查看。 */
    OSTaskSuspend(&AppTaskStartTCB, &err);
}

static void AppTaskCreate(void)
{
    OS_ERR err;

    OSTaskCreate(&AppTaskLed1TCB,
                 "LED1 Task",
                 AppTaskLed1,
                 (void *)0,
                 APP_TASK_LED1_PRIO,
                 &AppTaskLed1Stk[0],
                 APP_TASK_LED1_STK_SIZE / 10u,
                 APP_TASK_LED1_STK_SIZE,
                 0u,
                 0u,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);
    if (err != OS_ERR_NONE)
    {
        Error_Handler();
    }

    OSTaskCreate(&AppTaskLed2TCB,
                 "LED2 Task",
                 AppTaskLed2,
                 (void *)0,
                 APP_TASK_LED2_PRIO,
                 &AppTaskLed2Stk[0],
                 APP_TASK_LED2_STK_SIZE / 10u,
                 APP_TASK_LED2_STK_SIZE,
                 0u,
                 0u,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);
    if (err != OS_ERR_NONE)
    {
        Error_Handler();
    }

    OSTaskCreate(&AppTaskAht20TCB,
                 "AHT20 Task",
                 AppTaskAht20,
                 (void *)0,
                 APP_TASK_AHT20_PRIO,
                 &AppTaskAht20Stk[0],
                 APP_TASK_AHT20_STK_SIZE / 10u,
                 APP_TASK_AHT20_STK_SIZE,
                 0u,
                 0u,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);
    if (err != OS_ERR_NONE)
    {
        Error_Handler();
    }
}

/**
  * @brief  Task1：PC13 板载 LED，1 s 周期。
  */
static void AppTaskLed1(void *p_arg)
{
    OS_ERR err;

    (void)p_arg;

    LED1_OFF();

    while (DEF_TRUE)
    {
        LED1_ON();
        OSTimeDlyHMSM(0u, 0u, 0u, 500u,
                      (OS_OPT)(OS_OPT_TIME_HMSM_STRICT | OS_OPT_TIME_PERIODIC), &err);

        LED1_OFF();
        OSTimeDlyHMSM(0u, 0u, 0u, 500u,
                      (OS_OPT)(OS_OPT_TIME_HMSM_STRICT | OS_OPT_TIME_PERIODIC), &err);
    }
}

/**
  * @brief  Task2：PB12 外接 LED，3 s 周期。
  */
static void AppTaskLed2(void *p_arg)
{
    OS_ERR err;

    (void)p_arg;

    LED2_OFF();

    while (DEF_TRUE)
    {
        LED2_ON();
        OSTimeDlyHMSM(0u, 0u, 1u, 500u,
                      (OS_OPT)(OS_OPT_TIME_HMSM_STRICT | OS_OPT_TIME_PERIODIC), &err);

        LED2_OFF();
        OSTimeDlyHMSM(0u, 0u, 1u, 500u,
                      (OS_OPT)(OS_OPT_TIME_HMSM_STRICT | OS_OPT_TIME_PERIODIC), &err);
    }
}

/**
  * @brief  Task3：每 2 s 读一次 AHT20，并通过 USART1 输出。
  */
static void AppTaskAht20(void *p_arg)
{
    OS_ERR err;
    aht20_data_t data;
    bool init_ok;

    (void)p_arg;

    bsp_uart_puts("\r\n[TASK3] AHT20 init (7-bit address 0x38)...\r\n");
    init_ok = aht20_init();

    if (init_ok)
    {
        bsp_uart_puts("[TASK3] AHT20 init OK\r\n");
    }
    else
    {
        bsp_uart_puts("[TASK3] AHT20 init FAILED, check 3.3V/GND/PB6/PB7\r\n");
    }

    while (DEF_TRUE)
    {
        /* 任务要求：每 2 s 通过串口发送问候语 */
        bsp_uart_puts("hello uc/OS! 欢迎来到RTOS多任务环境！\r\n");

        if (aht20_read(&data))
        {
            bsp_uart_puts("[TASK3] AHT20: T=");
            uart_put_1dec(data.temperature);
            bsp_uart_puts(" C, RH=");
            uart_put_1dec(data.humidity);
            bsp_uart_puts(" %, CRC=");
            bsp_uart_puts(data.crc_ok ? "OK" : "ERR");
            bsp_uart_puts("\r\n");
        }
        else
        {
            bsp_uart_puts("[TASK3] AHT20 read failed, status=0x");
            bsp_uart_put_hex8(aht20_last_status());
            bsp_uart_puts("\r\n");
        }

        /* 周期 2 s，从本次唤醒时刻算起，自动扣除测量和串口输出耗时 */
        OSTimeDlyHMSM(0u, 0u, 2u, 0u,
                      (OS_OPT)(OS_OPT_TIME_HMSM_STRICT | OS_OPT_TIME_PERIODIC), &err);
    }
}
