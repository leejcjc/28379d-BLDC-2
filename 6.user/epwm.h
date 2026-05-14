/*
 * epwm.h
 *
 *  Created on: 2025年11月18日
 *      Author: Lenovo
 */

#ifndef EPWM_H_
#define EPWM_H_

//  PWM 参数定义
#define SYSCLK_FREQ_HZ      200000000UL // 系统时钟 200MHz
#define PWM_FREQ_HZ         20000UL     // PWM频率 20kHz
#define TBCLK_FREQ_HZ       50000000UL  // 时基时钟 50MHz (SYSCLK/2/2)

// TBPRD = 50,000,000 / (2 * 20,000) = 1250
#define BLDC_TBPRD_CYCLES   1250U

// Deadband Cycles = 1500ns * 50MHz = 75
#define BLDC_DEADBAND_CYCLES 75U

void BLDC_initEPWM_Single(uint32_t base);
void BLDC_initEPWM(void);
//void BLDC_setDuty(float duty);
//void BLDC_setDutySigned(float duty);

__interrupt void epwm1ISR(void);

#endif
