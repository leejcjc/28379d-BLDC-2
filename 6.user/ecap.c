/*
 * ecap.c
 *
 *  Created on: 2025年11月26日
 *      Author: Lenovo
 */

#include "driverlib.h"
#include "ecap.h"
#include "GPIO.h"

volatile uint32_t cap1Count1 = 0U;
volatile uint32_t cap1Count2 = 0U;
volatile uint32_t cap1Count3 = 0U;
volatile uint32_t cap1Count4 = 0U;

volatile float ecap1_dutyCycle;
volatile float ecap1_Fre_value;
volatile float Speed;

extern volatile uint32_t zero_speed_counter;

void initECAP1()
{
    XBAR_setInputPin(XBAR_INPUT7, 24);
//    GPIO_setPinConfig(GPIO_24_GPIO24);
//    GPIO_setDirectionMode(24, GPIO_DIR_MODE_IN);
//    GPIO_setPadConfig(24, GPIO_PIN_TYPE_PULLUP);
//    GPIO_setQualificationMode(24, GPIO_QUAL_ASYNC);

    //禁用 eCAP1 中断，并清除所有可能遗留的中断标志位和事件标志位，防止初始化未完成就触发中断
    ECAP_disableInterrupt(ECAP1_BASE,
                              (ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
                               ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
                               ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
                               ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
                               ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
                               ECAP_ISR_SOURCE_COUNTER_PERIOD   |
                               ECAP_ISR_SOURCE_COUNTER_COMPARE));
    ECAP_clearInterrupt(ECAP1_BASE,
                            (ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
                             ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
                             ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
                             ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
                             ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
                             ECAP_ISR_SOURCE_COUNTER_PERIOD   |
                             ECAP_ISR_SOURCE_COUNTER_COMPARE));

     // Disables time stamp capture.
     ECAP_disableTimeStampCapture(ECAP1_BASE);
     // Stops Time stamp counter.
     ECAP_stopCounter(ECAP1_BASE);
     // 设置 eCAP 为“捕获模式”（另一种是 APWM 输出模式）
     ECAP_enableCaptureMode(ECAP1_BASE);
     // Sets the capture mode.
     ECAP_setCaptureMode(ECAP1_BASE, ECAP_ONE_SHOT_CAPTURE_MODE, ECAP_EVENT_4);
     // 不分频
//     ECAP_setEventPrescaler(ECAP1_BASE, 0);

     ECAP_setEventPolarity(ECAP1_BASE, ECAP_EVENT_1, ECAP_EVNT_FALLING_EDGE);
     ECAP_setEventPolarity(ECAP1_BASE, ECAP_EVENT_2, ECAP_EVNT_RISING_EDGE);
     ECAP_setEventPolarity(ECAP1_BASE, ECAP_EVENT_3, ECAP_EVNT_FALLING_EDGE);
     ECAP_setEventPolarity(ECAP1_BASE, ECAP_EVENT_4, ECAP_EVNT_RISING_EDGE);

     ECAP_enableCounterResetOnEvent(ECAP1_BASE, ECAP_EVENT_1);
     ECAP_enableCounterResetOnEvent(ECAP1_BASE, ECAP_EVENT_2);
     ECAP_enableCounterResetOnEvent(ECAP1_BASE, ECAP_EVENT_3);
     ECAP_enableCounterResetOnEvent(ECAP1_BASE, ECAP_EVENT_4);



//     ECAP_enableLoadCounter(ECAP1_BASE);
     //禁用计数器加载，让eCAP忽略ePWM 发来的同步信号，自由计数
     ECAP_disableLoadCounter(ECAP1_BASE);
     //eCAP1仍然会将接收到的同步信号（SYNC In）透传输出给下一个模块（SYNC Out）
     ECAP_setSyncOutMode(ECAP1_BASE, ECAP_SYNC_OUT_SYNCI);

//     ECAP_setEmulationMode(ECAP1_BASE,ECAP_EMULATION_STOP);//打断点时，ecap停止计数


     ECAP_startCounter(ECAP1_BASE);
     ECAP_enableTimeStampCapture(ECAP1_BASE);
     ECAP_reArm(ECAP1_BASE);
     // Enables interrupt source for eCAP1.
     ECAP_enableInterrupt(ECAP1_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_4);


}

__interrupt void ecap1ISR(void)
{
    cap1Count1 = ECAP_getEventTimeStamp(ECAP1_BASE, ECAP_EVENT_1);//T1
    cap1Count2 = ECAP_getEventTimeStamp(ECAP1_BASE, ECAP_EVENT_2);//T2
    cap1Count3 = ECAP_getEventTimeStamp(ECAP1_BASE, ECAP_EVENT_3);//T3
    cap1Count4 = ECAP_getEventTimeStamp(ECAP1_BASE, ECAP_EVENT_4);//T4

    uint32_t total_period = cap1Count3 + cap1Count4;

    if (total_period > 0)//防止total_period=0的情况
    {
        ecap1_dutyCycle = (float)cap1Count3 / (float)total_period;

        ecap1_Fre_value = 200000000.0f / (float)total_period;

        Speed = ecap1_Fre_value * (60.0f / 14.0f);
    }


//    // Clear interrupt flags for more interrupts.
    ECAP_clearInterrupt(ECAP1_BASE,ECAP_ISR_SOURCE_CAPTURE_EVENT_4);
    ECAP_clearGlobalInterrupt(ECAP1_BASE);
//
//    // Re-arms the eCAP module for eCAP1.
    ECAP_reArm(ECAP1_BASE);
//    // Acknowledge the group interrupt for more interrupts.
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP4);

}



