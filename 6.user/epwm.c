/*
 * epwm.c
 *
 *  Created on: 2025年11月18日
 *      Author: Lenovo
 */

#include "driverlib.h"
#include "device.h"
#include "epwm.h"
#include "bldc.h"
#include "gpio.h"


volatile int i = 0;

//单个ePWM初始化
void BLDC_initEPWM_Single(uint32_t base)
{
    //TB
    // EPWMCLK = SYSCLK / 2 = 100MHz 系统默认
    // TBCLK = 100MHz / 2 = 50MHz
    // pwm频率=epwm时钟频率/（分频系数*TBPRD）=100MHz/2*1250

    EPWM_setTimeBasePeriod(base, BLDC_TBPRD_CYCLES);// 设置周期值 (TBPRD = 1250) 2500
    EPWM_setPhaseShift(base, 0);
    EPWM_setTimeBaseCounter(base, 0);// 清零计数器
    EPWM_setTimeBaseCounterMode(base, EPWM_COUNTER_MODE_UP_DOWN);// 设置计数模式为增减计数
    EPWM_disablePhaseShiftLoad(base);// 禁用相位加载
    EPWM_setClockPrescaler(base,
                           EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_2);

    //CC
    EPWM_setCounterCompareValue(base, EPWM_COUNTER_COMPARE_A, BLDC_TBPRD_CYCLES * 0.30f);//设置初始占空比30%
    EPWM_setCounterCompareShadowLoadMode(base, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);
    //再修改占空比时，不要立刻生效，要等到当前周期结束（计数器归0）时再通过 影子机制 更新，以防波形出错

    //AQ
    EPWM_setActionQualifierAction(base,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(base,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

    // 配置AQCSFRC寄存器
    EPWM_setActionQualifierContSWForceShadowMode(base, EPWM_AQ_SW_SH_LOAD_ON_CNTR_ZERO);
//    作用：配置 软件强制寄存器(AQCSFRC)的加载时机，
//    “当我通过AQCSFRC寄存器命令你强制输出A路为低电平时，请不要立即执行，而是等到下一次时基计数器TBCTR等于0时再生效。”



    //DB
    //选择死区的基准信号
    EPWM_setRisingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);
    EPWM_setFallingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);

    //死区时间
    EPWM_setFallingEdgeDelayCount(base, 75);// 75 * ( 1 / 50MHz ) = 1500ns = 1.5us   75/200MHz
    EPWM_setRisingEdgeDelayCount(base, 75);//
    //互补
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);//上升沿，高电平有效 不翻转
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);//下降沿，低电平有效 翻转 产生互补波形

    EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, true);// 使能上升沿(RED)和下降沿(FED)延迟
    EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, true);

    BLDC_setPhaseAction(base, ACTION_HIZ);// 默认设置为高阻态, 等待换相指令

    i++;
}

void BLDC_initEPWM(void)
{
//    Interrupt_register(INT_EPWM1, &epwm1ISR);

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM2);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM3);

    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    BLDC_initEPWM_Single(EPWM1_BASE);
    BLDC_initEPWM_Single(EPWM2_BASE);
    BLDC_initEPWM_Single(EPWM3_BASE);

    //实际上有无信号同步都可以运行，但理论上是需要加上的，保险一点
    EPWM_disablePhaseShiftLoad(EPWM1_BASE);//主机不用相位加载
    EPWM_setSyncOutPulseMode(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);//发出脉冲

    EPWM_enablePhaseShiftLoad(EPWM2_BASE);//启用相位加载，接收同步信号
    EPWM_setPhaseShift(EPWM2_BASE, 0);//设置相位值 0表示同相同步
    EPWM_setSyncOutPulseMode(EPWM2_BASE, EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);//设置同步输出源 透传

    EPWM_enablePhaseShiftLoad(EPWM3_BASE);
    EPWM_setPhaseShift(EPWM3_BASE, 0);
    EPWM_setSyncOutPulseMode(EPWM3_BASE, EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);

    //中断触发
    EPWM_setInterruptSource(EPWM1_BASE, EPWM_INT_TBCTR_ZERO);
    EPWM_setInterruptEventCount(EPWM1_BASE, 1);
    EPWM_enableInterrupt(EPWM1_BASE);

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
}




