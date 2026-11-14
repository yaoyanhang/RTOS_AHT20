/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   中断服务函数。
  *
  * SysTick 同时承担两个职责：
  *   1) HAL_IncTick()：维持 HAL_GetTick()/HAL_Delay() 的 1 ms 计数；
  *   2) OS_CPU_SysTickHandler()：uC/OS-III 的系统节拍和任务延时。
  *
  * OSStart() 之前只执行 HAL_IncTick()，避免内核尚未启动时进入 OSTimeTick()。
  ******************************************************************************
  */
#include "main.h"
#include "includes.h"

void NMI_Handler(void)
{
    while (1)
    {
    }
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

/**
  * @brief  uC/OS-III 的 PendSV 入口在 os_cpu_a.s 中实现，
  *         启动文件的向量表已直接指向 OS_CPU_PendSVHandler，这里不再定义。
  */

void SysTick_Handler(void)
{
    HAL_IncTick();

    if (OSRunning == OS_STATE_OS_RUNNING)
    {
        OS_CPU_SysTickHandler();
    }
}
