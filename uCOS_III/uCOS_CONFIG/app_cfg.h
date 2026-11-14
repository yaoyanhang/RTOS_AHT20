/**
  ******************************************************************************
  * @file    app_cfg.h
  * @brief   uC/OS-III 应用配置（本工程只保留任务、节拍和延时功能）。
  ******************************************************************************
  */
#ifndef APP_CFG_H
#define APP_CFG_H

/* 应用任务优先级（数值越小优先级越高） */
#define APP_TASK_START_PRIO          3u
#define APP_TASK_LED1_PRIO           4u
#define APP_TASK_LED2_PRIO           5u
#define APP_TASK_AHT20_PRIO          6u

/* 任务栈大小，单位：CPU_STK 元素（STM32 上 1 个元素 = 4 字节） */
#define APP_TASK_START_STK_SIZE      128u
#define APP_TASK_LED1_STK_SIZE       128u
#define APP_TASK_LED2_STK_SIZE       128u
#define APP_TASK_AHT20_STK_SIZE      384u

#endif /* APP_CFG_H */
