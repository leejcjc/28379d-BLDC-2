/*
 * main.c
 *
 *  Created on: 2025年11月12日
 *      Author: Lenovo
 */

#include "driverlib.h"
#include "device.h"
#include "epwm.h"
#include "bldc.h"
#include "gpio.h"
#include "ecap.h"


volatile uint32_t mainLoopCount = 0;
float targetDuty = 0.0f; // 启动占空比
bool rampingUp = true;
float step = 0.01f;
extern volatile float Speed;
int epwm1ISRcount = 0;


//位置环
volatile long Target_Pos = 0;      // 目标位置（比如 840 代表转 10 圈）
float kp_pos = 2.0f;               // P参数：以多少速度来修正一个误差
#define POS_DEADBAND 1             // 死区：允许1个步进的误差（约4.3度）
#define MAX_SPEED_REF 1000.0f      // 最大速度
extern volatile long g_HallPositionCnt ;

//速度环
float kp_v = 0.0005f;
float ki_v = 0.009f;
volatile float Speed_ref = 0;
float err_Speed = 0;
float int_err_Speed = 0;
float Ts = 0.001f;
volatile float Speed_Filtered = 0.0f;
float pid_duty;



void main(void)
{
    Device_init();
    Device_initGPIO();
    Interrupt_initModule();//初始化PIE并清除PIE寄存器,禁用CPU中断。
    Interrupt_initVectorTable();//初始化PIE向量表

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_ECAP1);

    BLDC_initGPIO();
    BLDC_initEPWM();
    BLDC_initHallInterrupts();

    Interrupt_register(INT_EPWM1, &epwm1ISR);
    Interrupt_register(INT_ECAP1, &ecap1ISR);
    initECAP1();
    Interrupt_enable(INT_ECAP1);
    Interrupt_enable(INT_EPWM1);


    //使能全局中断
    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM


    //启动电机
    dutyCycle = targetDuty;
//    dutyCycle = 0.1f;

        g_HallPositionCnt = 0; // 上电归零
//        Target_Pos = 840;      // 设定目标

    BLDC_startMotor();      //启动换相和驱动器

    DEVICE_DELAY_US(50000);

    while(1)
    {
        mainLoopCount++;
    }

}


__interrupt void epwm1ISR(void)
{
    //PWM 频率 = 20kHz -> 周期 50us
    //PID 周期 = 50us * 20 = 1000us = 1ms
    epwm1ISRcount++;

    static uint16_t loop_cnt = 0;

    if(++loop_cnt >= 20)           //分频20，1kHz跑pid，相当于50us*20=1000us才进行一次pid计算
    {
        loop_cnt = 0;

        //----------------------位置环----------------------
        long pos_error = Target_Pos - g_HallPositionCnt; //计算位置误差
        float speed_command = 0.0f;
        float abs_pos_error;

        if(pos_error > 0) abs_pos_error = pos_error;
        if(pos_error < 0) abs_pos_error = -pos_error; //绝对值

        if (abs_pos_error <= POS_DEADBAND)//死区
        {
            pos_error = 0;
            speed_command = 0.0f;
            int_err_Speed = 0.0f;//到达目标后，清空速度环的积分项
        }
        else
        {
            //纯P
            speed_command = kp_pos * (float)pos_error;

            //限幅
            if (speed_command > MAX_SPEED_REF) speed_command = MAX_SPEED_REF;
            if (speed_command < -MAX_SPEED_REF) speed_command = -MAX_SPEED_REF;
        }

        Speed_ref = speed_command;
        //--------------------------------------------


        //速度环PI

        //带符号的真实速度
        float feedback_speed_signed = Speed;

        //根据当前电机的物理方向，手动补上负号
        if (currentDirection == DIR_REVERSE)
        {
            feedback_speed_signed = -feedback_speed_signed;
        }

        err_Speed = Speed_ref - feedback_speed_signed;
        int_err_Speed = int_err_Speed + (err_Speed * Ts) ;//积分误差

        pid_duty = (kp_v * err_Speed) + (ki_v * int_err_Speed);

        //积分限幅
        //limit = max_output / ki_v
        if(ki_v > 0)
        {
            float max_i = 0.7f / ki_v;
            if (int_err_Speed > max_i)
            {
                int_err_Speed = max_i;
            }
            if (int_err_Speed < -max_i)
            {
                int_err_Speed = -max_i;
            }
        }


        //输出限幅
        if (pid_duty > 0.7f)
        {
            pid_duty = 0.7f;
        }
        if (pid_duty < -0.7f )
        {
            pid_duty = -0.7f;
        }

            if (pid_duty > -0.02f && pid_duty < 0.02f)
            {
                pid_duty = 0.0f; // 强制归零
            }


        BLDC_setDutySigned(pid_duty);
    }

    // Clear INT flag for this timer
    EPWM_clearEventTriggerInterruptFlag(EPWM1_BASE);

    // Acknowledge interrupt group
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}





