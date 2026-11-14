/**
  ******************************************************************************
  * @file    board.h
  * @brief   STM32F103C8T6 最小系统板引脚定义。
  *
  * LED1：PC13，板载 LED，低电平点亮，1 s 周期。
  * LED2：PB12，外接 LED（PB12 -> 330 Ω -> LED -> GND），高电平点亮，3 s 周期。
  ******************************************************************************
  */
#ifndef BOARD_H
#define BOARD_H

#include "stm32f1xx_hal.h"

/* ---------------- LED1：PC13，低电平点亮 ---------------- */
#define LED1_PORT               GPIOC
#define LED1_PIN                GPIO_PIN_13

#define LED1_ON()               HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET)
#define LED1_OFF()              HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET)
#define LED1_TOGGLE()           HAL_GPIO_TogglePin(LED1_PORT, LED1_PIN)

/* ---------------- LED2：PB12，高电平点亮 ---------------- */
#define LED2_PORT               GPIOB
#define LED2_PIN                GPIO_PIN_12

#define LED2_ON()               HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_SET)
#define LED2_OFF()              HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_RESET)
#define LED2_TOGGLE()           HAL_GPIO_TogglePin(LED2_PORT, LED2_PIN)

#endif /* BOARD_H */
