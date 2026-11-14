/**
  ******************************************************************************
  * @file    app.h
  * @brief   uC/OS-III 应用层：三个任务的声明和初始化入口。
  ******************************************************************************
  */
#ifndef APP_H
#define APP_H

#include "includes.h"

/* 任务优先级：数值越小优先级越高 */
#define APP_TASK_START_PRIO       3u
#define APP_TASK_LED1_PRIO        4u
#define APP_TASK_LED2_PRIO        5u
#define APP_TASK_AHT20_PRIO       6u

/* 任务栈大小，单位：CPU_STK（STM32 上 1 个 = 4 字节） */
#define APP_TASK_START_STK_SIZE   128u
#define APP_TASK_LED1_STK_SIZE    128u
#define APP_TASK_LED2_STK_SIZE    128u
#define APP_TASK_AHT20_STK_SIZE   384u

extern OS_TCB  AppTaskStartTCB;
extern OS_TCB  AppTaskLed1TCB;
extern OS_TCB  AppTaskLed2TCB;
extern OS_TCB  AppTaskAht20TCB;

void App_Init(void);
void AppTaskStart(void *p_arg);

#endif /* APP_H */
