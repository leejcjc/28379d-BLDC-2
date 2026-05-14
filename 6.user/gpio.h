/*
 * gpio.h
 *
 *  Created on: 2025年11月18日
 *      Author: Lenovo
 */

#include "driverlib.h"
#include "device.h"

#ifndef GPIO_H_
#define GPIO_H_

void BLDC_initGPIO(void);

// ePWM 引脚
#define BLDC_EPWM1A_GPIO    0U   // U相上桥 GPIO0
#define BLDC_EPWM1B_GPIO    1U   // U相下桥 GPIO1
#define BLDC_EPWM2A_GPIO    2U   // V相上桥 GPIO2
#define BLDC_EPWM2B_GPIO    3U   // V相下桥 GPIO3
#define BLDC_EPWM3A_GPIO    4U   // W相上桥 GPIO4
#define BLDC_EPWM3B_GPIO    5U   // W相下桥 GPIO5

//ePWM引脚功能复用定义
#define BLDC_EPWM1A_CONFIG  GPIO_0_EPWM1A
#define BLDC_EPWM1B_CONFIG  GPIO_1_EPWM1B
#define BLDC_EPWM2A_CONFIG  GPIO_2_EPWM2A
#define BLDC_EPWM2B_CONFIG  GPIO_3_EPWM2B
#define BLDC_EPWM3A_CONFIG  GPIO_4_EPWM3A
#define BLDC_EPWM3B_CONFIG  GPIO_5_EPWM3B

//霍尔引脚
#define BLDC_HALL_U_GPIO    24U  //U GPIO24
#define BLDC_HALL_V_GPIO    25U  //V GPIO25
#define BLDC_HALL_W_GPIO    26U  //W GPIO26

// 驱动器使能引脚
#define BLDC_ENABLE_GPIO    9U
#define GPIO_CFG_DRIVER_EN GPIO_9_GPIO9


#endif

