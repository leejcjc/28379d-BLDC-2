/*
 * led.c
 *
 *  Created on: 2025年11月12日
 *      Author: Lenovo
 */


#include "driverlib.h"
#include "led.h"

void LED_Init(void)
{

    GPIO_setPinConfig(GPIO_CFG_LED1);//设置引脚配置 为普通GPIO
    GPIO_setMasterCore(GPIO_PIN_LED1, GPIO_CORE_CPU1);//用于CPU1
    GPIO_setPadConfig(GPIO_PIN_LED1, GPIO_PIN_TYPE_STD);//推挽输出或浮动输入
    GPIO_setDirectionMode(GPIO_PIN_LED1, GPIO_DIR_MODE_OUT);//GPIO 为输出模式

    GPIO_setPinConfig(GPIO_CFG_LED2);//设置引脚配置 为普通GPIO
    GPIO_setMasterCore(GPIO_PIN_LED2, GPIO_CORE_CPU1);//用于CPU1
    GPIO_setPadConfig(GPIO_PIN_LED2, GPIO_PIN_TYPE_STD);//推挽输出或浮动输入
    GPIO_setDirectionMode(GPIO_PIN_LED2, GPIO_DIR_MODE_OUT);//GPIO 为输出模式

}
