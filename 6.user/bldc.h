/*
 * bldc.h
 *
 *  Created on: 2025年11月12日
 *      Author: Lenovo
 */

#ifndef BLDC_H_
#define BLDC_H_

#include "driverlib.h"
#include "device.h"

extern volatile uint16_t hallState;//霍尔状态
extern volatile float dutyCycle;//当前的目标占空比
extern volatile bool motorEnabled;//电机使能标志

void BLDC_initGPIO(void);
void BLDC_initHallInterrupts(void);
void BLDC_setDuty(float duty);
void BLDC_stopMotor(void);
void BLDC_startMotor(void);
void BLDC_commutation(uint16_t hall);
void BLDC_setPhaseAction(uint32_t base, uint16_t action);

void BLDC_setDutySigned(float duty);
void BLDC_setDuty(float duty);

__interrupt void BLDC_hallIsr1(void);
__interrupt void BLDC_hallIsr2(void);
__interrupt void BLDC_hallIsr3(void);

#define ACTION_HIZ      0  // 高阻态
#define ACTION_PWM_HI   1  // PWM高电平有效
#define ACTION_GND      2  // 强制拉低

//---反向---
typedef enum {
    DIR_FORWARD = 1,
    DIR_REVERSE = -1
} MotorDir_t;
extern volatile MotorDir_t currentDirection;



#endif // BLDC_H_


