/*
 * bldc.c
 *
 *  Created on: 2025年11月12日
 *      Author: Lenovo
 */
#include "driverlib.h"
#include "device.h"
#include "bldc.h"
#include "GPIO.h"
#include "epwm.h"

volatile MotorDir_t currentDirection = DIR_FORWARD; // 默认为正向


volatile uint16_t hallState = 0;
volatile float dutyCycle = 0.0f; // 默认0%占空比起步
volatile bool motorEnabled = false;
volatile int BLDC_startMotor1 = 0;
volatile int BLDC_stopMotor1 = 0;
volatile int BLDC_hallChangeHandler1_commutation = 0;
volatile int BLDC_hallIsr1a = 0;
volatile int BLDC_hallIsr2a = 0;
volatile int BLDC_hallIsr3a = 0;

volatile long g_HallPositionCnt = 0;


//(W V U)  {U相动作, V相动作, W相动作}
const uint16_t commutationTable_FWD[8][3] =
{
    //000
    {0, 0, 0},

    //001 (U+ W-)  1
    {1, 0,  2},

    //010 (V+ U-)  2
    {2,  1, 0},

    //011 (V+ W-)  3
    {0,  1, 2},

    //100 (W+ V-)  4
    {0,  2,  1},

    //101 (U+ V-)  5
    {1, 2,  0},

    //110 (W+ U-)  6
    {2,  0,  1},

    //111
    {0, 0, 0}
};

//---反向
const uint16_t commutationTable_REV[8][3] =
{
    //000
    {0, 0, 0},
    {2, 0, 1},
    {1, 2, 0},
    {0, 2, 1},
    {0, 1, 2},
    {2, 1, 0},
    {1, 0, 2},
    {0, 0, 0}
};


//霍尔方向查找表(120度霍尔顺序: 5-1-3-2-6-4)
//行是Old_Hall, 列是New_Hall
const int8_t Hall_Dir_Table[8][8] =
{
//New:0   1   2   3   4   5   6   7     // Old
    { 0,  0,  0,  0,  0,  0,  0,  0 },  // 0 (无效)
    { 0,  0,  0,  1,  0, -1,  0,  0 },  // 1 -> 3(+), 1 -> 5(-)
    { 0,  0,  0, -1,  0,  0,  1,  0 },  // 2 -> 6(+), 2 -> 3(-)
    { 0, -1,  1,  0,  0,  0,  0,  0 },  // 3 -> 2(+), 3 -> 1(-)
    { 0,  0,  0,  0,  0,  1, -1,  0 },  // 4 -> 5(+), 4 -> 6(-)
    { 0,  1,  0,  0, -1,  0,  0,  0 },  // 5 -> 1(+), 5 -> 4(-)
    { 0,  0, -1,  0,  1,  0,  0,  0 },  // 6 -> 4(+), 6 -> 2(-)
    { 0,  0,  0,  0,  0,  0,  0,  0 }   // 7 (无效)
};

// 记录上一次的霍尔状态
volatile uint16_t preHallState = 0;


void BLDC_setPhaseAction(uint32_t base, uint16_t action)
{
    switch(action)
    {
        case ACTION_PWM_HI: //上桥PWM, 下桥互补PWM
            EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, true);
            EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, true);
            //移除软件强制
            EPWM_setActionQualifierContSWForceAction(base, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_DISABLED);
            break;

        case ACTION_GND: //上桥关, 下桥开
            EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, true);
            EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, true);
           //软件强制A低，B互补为高
           EPWM_setActionQualifierContSWForceAction(base, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);
            break;

        case ACTION_HIZ: //高阻态
        default:
            //禁用死区模块，AB独立
            EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, false);
            EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, false);
            //强制AB为低
            EPWM_setActionQualifierContSWForceAction(base, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);
            EPWM_setActionQualifierContSWForceAction(base, EPWM_AQ_OUTPUT_B, EPWM_AQ_SW_OUTPUT_LOW);
            break;
    }
}

//换相逻辑执行函数
void BLDC_commutation(uint16_t hall)
{
    // 检查是否为非法状态
    if((hall == 0) || (hall == 7))
    {
        BLDC_stopMotor();
        return;
    }
    //---反向
    uint16_t u_action, v_action, w_action;

    if(currentDirection == DIR_FORWARD)
        {
            // 使用原有的正转表 (假设你原来的表叫 commutationTable)
            u_action = commutationTable_FWD[hall][0];
            v_action = commutationTable_FWD[hall][1];
            w_action = commutationTable_FWD[hall][2];
        }
        else // DIR_REVERSE
        {
            // 使用反转表
            u_action = commutationTable_REV[hall][0];
            v_action = commutationTable_REV[hall][1];
            w_action = commutationTable_REV[hall][2];
        }

//    // 从查找表获取U,V,W三相的对应动作
//    uint16_t u_action = commutationTable[hall][0];
//    uint16_t v_action = commutationTable[hall][1];
//    uint16_t w_action = commutationTable[hall][2];

    // 应用动作到ePWM模块
    BLDC_setPhaseAction(EPWM1_BASE, u_action);
    BLDC_setPhaseAction(EPWM2_BASE, v_action);
    BLDC_setPhaseAction(EPWM3_BASE, w_action);
}


//霍尔中断初始化
void BLDC_initHallInterrupts(void)
{
    GPIO_setInterruptPin(BLDC_HALL_U_GPIO, GPIO_INT_XINT1);
    GPIO_setInterruptPin(BLDC_HALL_V_GPIO, GPIO_INT_XINT2);
    GPIO_setInterruptPin(BLDC_HALL_W_GPIO, GPIO_INT_XINT3);

    // 中断双边沿触发
    GPIO_setInterruptType(GPIO_INT_XINT1, GPIO_INT_TYPE_BOTH_EDGES);
    GPIO_setInterruptType(GPIO_INT_XINT2, GPIO_INT_TYPE_BOTH_EDGES);
    GPIO_setInterruptType(GPIO_INT_XINT3, GPIO_INT_TYPE_BOTH_EDGES);

    GPIO_enableInterrupt(GPIO_INT_XINT1);
    GPIO_enableInterrupt(GPIO_INT_XINT2);
    GPIO_enableInterrupt(GPIO_INT_XINT3);

    // 注册 ISR 函数
    Interrupt_register(INT_XINT1, &BLDC_hallIsr1);
    Interrupt_register(INT_XINT2, &BLDC_hallIsr2);
    Interrupt_register(INT_XINT3, &BLDC_hallIsr3);

    // 使能PIE中的XINT中断
    Interrupt_enable(INT_XINT1);
    Interrupt_enable(INT_XINT2);
    Interrupt_enable(INT_XINT3);
}


// duty：-0.95 到 0.95
void BLDC_setDutySigned(float duty)
{
    // 防止 PID 在 0 附近产生的微小噪音 (-0.0001) 导致频繁切换方向
    if (duty > -0.005f && duty < 0.005f)
    {
        duty = 0.0f;
    }

    MotorDir_t newDirection;
    float absduty;

    if(duty >= 0.0f)
    {
        newDirection = DIR_FORWARD;
        absduty = duty;
    }
    else
    {
        newDirection = DIR_REVERSE;
        absduty = -duty; //取绝对值，因为占空比都给正，只是按照不同的换相表运作
    }

    //如果方向突然反向，立即更新换相状态
    //否则在下一个霍尔信号到来前，电机还在用旧方向的逻辑跑，会导致短路或震动。
    if(newDirection != currentDirection)
    {
        currentDirection = newDirection;

        uint16_t u = GPIO_readPin(BLDC_HALL_U_GPIO);
        uint16_t v = GPIO_readPin(BLDC_HALL_V_GPIO);
        uint16_t w = GPIO_readPin(BLDC_HALL_W_GPIO);
        uint16_t currentHall = (w << 2) | (v << 1) | u;

        if(motorEnabled)
        {
            BLDC_commutation(currentHall);
        }
    }

    //限制占空比范围
    if(absduty > 0.95f) absduty = 0.95f;
//    if(duty < -0.95f) duty = -0.95f;

    dutyCycle = duty;

    uint16_t compareValue = (uint16_t)((float)BLDC_TBPRD_CYCLES * absduty);

    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, compareValue);
    EPWM_setCounterCompareValue(EPWM2_BASE, EPWM_COUNTER_COMPARE_A, compareValue);
    EPWM_setCounterCompareValue(EPWM3_BASE, EPWM_COUNTER_COMPARE_A, compareValue);

}



//控制占空比
void BLDC_setDuty(float duty)
{
    // 限制占空比在 0.0 到 0.95；防止100%占空比
    if(duty > 0.95f) duty = 0.95f;
    if(duty < 0.0f) duty = 0.0f;

    dutyCycle = duty; // 更新全局变量

    uint16_t compareValue = (uint16_t)((float)BLDC_TBPRD_CYCLES * dutyCycle);

    // 更新三相的比较寄存器
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, compareValue);
    EPWM_setCounterCompareValue(EPWM2_BASE, EPWM_COUNTER_COMPARE_A, compareValue);
    EPWM_setCounterCompareValue(EPWM3_BASE, EPWM_COUNTER_COMPARE_A, compareValue);
}


void BLDC_stopMotor(void)
{
    BLDC_stopMotor1++;

    motorEnabled = false;

    // 禁用驱动板
    GPIO_writePin(BLDC_ENABLE_GPIO, 0);

    // 将所有ePWM输出设置为高阻态
    BLDC_setPhaseAction(EPWM1_BASE, ACTION_HIZ);
    BLDC_setPhaseAction(EPWM2_BASE, ACTION_HIZ);
    BLDC_setPhaseAction(EPWM3_BASE, ACTION_HIZ);
}

void BLDC_startMotor(void)
{
    BLDC_startMotor1++;

    GPIO_writePin(BLDC_ENABLE_GPIO, 1);
    BLDC_setPhaseAction(EPWM1_BASE, ACTION_GND);//给下桥充电，让电机启动时可以克制静摩擦力
    BLDC_setPhaseAction(EPWM2_BASE, ACTION_GND);
    BLDC_setPhaseAction(EPWM3_BASE, ACTION_GND);
    DEVICE_DELAY_US(2000);

//    //切断所有 MOS 管，下桥完全关闭
//    BLDC_setPhaseAction(EPWM1_BASE, ACTION_HIZ);
//    BLDC_setPhaseAction(EPWM2_BASE, ACTION_HIZ);
//    BLDC_setPhaseAction(EPWM3_BASE, ACTION_HIZ);
//    DEVICE_DELAY_US(20);

    //设置占空比
    BLDC_setDutySigned(dutyCycle);

    //立即读取一次霍尔状态
    uint16_t u = GPIO_readPin(BLDC_HALL_U_GPIO);
    uint16_t v = GPIO_readPin(BLDC_HALL_V_GPIO);
    uint16_t w = GPIO_readPin(BLDC_HALL_W_GPIO);
    hallState = (w << 2) | (v << 1) | u;

    //执行第一次换相
    BLDC_commutation(hallState);

    //使能电机
    motorEnabled = true;

    //使能驱动板
    GPIO_writePin(BLDC_ENABLE_GPIO, 1);
}

//中断服务程序
void BLDC_hallChangeHandler(void)
{
    // 读取霍尔状态
    uint16_t u = GPIO_readPin(BLDC_HALL_U_GPIO);
    uint16_t v = GPIO_readPin(BLDC_HALL_V_GPIO);
    uint16_t w = GPIO_readPin(BLDC_HALL_W_GPIO);
    hallState = (w << 2) | (v << 1) | u;

    //-------位置环参数---------
    if(motorEnabled)
    {
//        if (currentDirection == DIR_FORWARD)
//                {
//                    g_HallPositionCnt++;
//                }
//                else
//                {
//                    g_HallPositionCnt--;
//                }

        // 第一次运行时，preHallState可能是0，需要初始化一下
                if (preHallState == 0) preHallState = hallState;

                // 查表获取方向增量 (+1, -1, 0)
                int8_t direction_step = Hall_Dir_Table[preHallState][hallState];

                // 更新全局位置计数器
                g_HallPositionCnt += direction_step;

                // 更新历史状态
                preHallState = hallState;


        // 执行换相
        BLDC_hallChangeHandler1_commutation++;
        BLDC_commutation(hallState);
    }
}

// XINT1 (PIE Group 1)
__interrupt void BLDC_hallIsr1(void)
{
    BLDC_hallIsr1a++;
//    BLDC_setDutySigned(dutyCycle);
    BLDC_hallChangeHandler();

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

// XINT2 (PIE Group 1)
__interrupt void BLDC_hallIsr2(void)
{
    BLDC_hallIsr2a++;
//    BLDC_setDutySigned(dutyCycle);
    BLDC_hallChangeHandler();

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

// XINT3 (PIE Group 12)
__interrupt void BLDC_hallIsr3(void)
{
    BLDC_hallIsr3a++;
//    BLDC_setDutySigned(dutyCycle);
    BLDC_hallChangeHandler();

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP12);
}


