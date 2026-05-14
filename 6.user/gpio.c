/*
 * gpio.c
 *
 *  Created on: 2025年11月18日
 *      Author: Lenovo
 */

#include "driverlib.h"
#include "device.h"
#include "GPIO.h"

void BLDC_initGPIO(void)
{
    //PWM 引脚
    GPIO_setPinConfig(BLDC_EPWM1A_CONFIG);
    GPIO_setPinConfig(BLDC_EPWM1B_CONFIG);
    GPIO_setPinConfig(BLDC_EPWM2A_CONFIG);
    GPIO_setPinConfig(BLDC_EPWM2B_CONFIG);
    GPIO_setPinConfig(BLDC_EPWM3A_CONFIG);
    GPIO_setPinConfig(BLDC_EPWM3B_CONFIG);

    GPIO_setPadConfig(BLDC_EPWM1A_GPIO,GPIO_PIN_TYPE_STD);
    GPIO_setPadConfig(BLDC_EPWM1B_GPIO,GPIO_PIN_TYPE_STD);
    GPIO_setPadConfig(BLDC_EPWM2A_GPIO,GPIO_PIN_TYPE_STD);
    GPIO_setPadConfig(BLDC_EPWM2B_GPIO,GPIO_PIN_TYPE_STD);
    GPIO_setPadConfig(BLDC_EPWM3A_GPIO,GPIO_PIN_TYPE_STD);
    GPIO_setPadConfig(BLDC_EPWM3B_GPIO,GPIO_PIN_TYPE_STD);

    GPIO_setQualificationMode(BLDC_EPWM1A_GPIO,GPIO_QUAL_SYNC);
    GPIO_setQualificationMode(BLDC_EPWM1B_GPIO,GPIO_QUAL_SYNC);
    GPIO_setQualificationMode(BLDC_EPWM2A_GPIO,GPIO_QUAL_SYNC);
    GPIO_setQualificationMode(BLDC_EPWM2B_GPIO,GPIO_QUAL_SYNC);
    GPIO_setQualificationMode(BLDC_EPWM3A_GPIO,GPIO_QUAL_SYNC);
    GPIO_setQualificationMode(BLDC_EPWM3B_GPIO,GPIO_QUAL_SYNC);

        //霍尔引脚
        // 修改霍尔引脚配置，使用采样滤波
        // 采样周期 = 2 * SYSCLKOUT
        // 采样 6 次电平一致才认为是有效信号，滤除尖峰干扰

        GPIO_setQualificationMode(BLDC_HALL_U_GPIO, GPIO_QUAL_6SAMPLE);
        GPIO_setQualificationPeriod(BLDC_HALL_U_GPIO, 2); // 采样间隔

        GPIO_setQualificationMode(BLDC_HALL_V_GPIO, GPIO_QUAL_6SAMPLE);
        GPIO_setQualificationPeriod(BLDC_HALL_V_GPIO, 2);

        GPIO_setQualificationMode(BLDC_HALL_W_GPIO, GPIO_QUAL_6SAMPLE);
        GPIO_setQualificationPeriod(BLDC_HALL_W_GPIO, 2);


    GPIO_setPinConfig(GPIO_24_GPIO24);
    GPIO_setDirectionMode(BLDC_HALL_U_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(24, GPIO_PIN_TYPE_PULLUP);//有无上拉都可以运行，只不过加上更保险
//    GPIO_setQualificationMode(BLDC_HALL_U_GPIO, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_25_GPIO25);
    GPIO_setDirectionMode(BLDC_HALL_V_GPIO, GPIO_DIR_MODE_IN);
//    GPIO_setQualificationMode(BLDC_HALL_V_GPIO, GPIO_QUAL_SYNC);

    GPIO_setPinConfig(GPIO_26_GPIO26);
    GPIO_setDirectionMode(BLDC_HALL_W_GPIO, GPIO_DIR_MODE_IN);
//    GPIO_setQualificationMode(BLDC_HALL_W_GPIO, GPIO_QUAL_SYNC);

//        //启用内部上拉电阻-----上拉电阻仅用于“手动转动转子实验-验证霍尔逻辑”，霍尔传感器是开漏的，并且它们需要上拉电阻才能工作。
//        GPIO_setPadConfig(BLDC_HALL_U_GPIO, GPIO_PIN_TYPE_PULLUP);
//        GPIO_setPadConfig(BLDC_HALL_V_GPIO, GPIO_PIN_TYPE_PULLUP);
//        GPIO_setPadConfig(BLDC_HALL_W_GPIO, GPIO_PIN_TYPE_PULLUP);

    //驱动器使能引脚
    GPIO_setPinConfig(GPIO_9_GPIO9);
    GPIO_setDirectionMode(BLDC_ENABLE_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(BLDC_ENABLE_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_writePin(BLDC_ENABLE_GPIO, 0); // 默认禁用

}


