/*
 * led.h
 *
 *  Created on: 2025年11月12日
 *      Author: Lenovo
 */

#ifndef USER_LED_H_
#define USER_LED_H_

#define GPIO_PIN_LED1        31U  // GPIO number for LD10
#define GPIO_PIN_LED2        34U  // GPIO number for LD9
#define GPIO_CFG_LED1        GPIO_31_GPIO31  // "pinConfig" for LD10
#define GPIO_CFG_LED2        GPIO_34_GPIO34  // "pinConfig" for LD9

#define LED1_OFF GPIO_writePin(GPIO_PIN_LED1,1) //高电平
#define LED1_ON  GPIO_writePin(GPIO_PIN_LED1,0) //低电平
#define LED1_TOGGLE GPIO_togglePin(GPIO_PIN_LED1);//翻转

#define LED2_OFF GPIO_writePin(GPIO_PIN_LED2,1) //高电平
#define LED2_ON  GPIO_writePin(GPIO_PIN_LED2,0) //低电平
#define LED2_TOGGLE GPIO_togglePin(GPIO_PIN_LED2);//翻转


void LED_Init(void);

#endif /* USER_LED_H_ */

