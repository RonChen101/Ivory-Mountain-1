#include "RTX51TNY.H"
#include "config.h"
#include "GPIO.h"
#include "UART.h"
#include "NVIC.h"
#include "Switch.h"
#include "STC8H_PWM.h"

/**************************************

通过RTX51系统实现多个任务的独立运行(并发, 区别并行)

任务0: 初始化外设, 创建其他任务, 销毁自己
任务1: 串口2(蓝牙)接收，更新按键/摇杆状态
任务2: 舵机控制

P2.6 舵机（PWM4/PWMA）：
    长按 A 键持续正转，长按 B 键持续反转
P0.2 舵机（PWM7/PWMB）：
    摇杆推哪转到哪（方向已反转），松手后固定
D 键：
    两个舵机同时缓慢复位

***************************************/

#define PREESCALER  10
#define FREQ        50
#define PERIOD (MAIN_Fosc / FREQ / PREESCALER)

/* ===== 两个舵机的复位角度 ===== */
#define ANGLE_A_INIT   45
#define ANGLE_B_INIT   90

/* ===== 摇杆中点与死区 ===== */
#define JOY_MID    0x00
#define JOY_DEAD   10

PWMx_Duty dutyA = {0};     // PWMA，PWM4 -> P2.6
PWMx_Duty dutyB = {0};     // PWMB，PWM7 -> P0.2

float angleA   = ANGLE_A_INIT;   // P2.6 舵机当前角度，0~90°
float angleB   = ANGLE_B_INIT;   // P0.2 舵机当前角度，0~180°
float target_B = ANGLE_B_INIT;   // P0.2 舵机目标角度

/* ========== 全局状态 ========== */
u8 flag_A = 0;
u8 flag_B = 0;
u8 flag_D = 0;
u8 flag_reset = 0;          // 复位进行中标志
u8 joy_x  = JOY_MID;

void GPIO_config(void) {
    GPIO_InitTypeDef	GPIO_InitStructure;
    // UART1 P30 P31 准双向
    GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.Mode = GPIO_PullUp;
    GPIO_Inilize(GPIO_P3, &GPIO_InitStructure);

    // UART2 P10 P11 准双向
    GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.Mode = GPIO_PullUp;
    GPIO_Inilize(GPIO_P1, &GPIO_InitStructure);

    // P2.6 推挽输出（PWM4）
    P2M1 &= ~0x40;
    P2M0 |=  0x40;

    // P0.2 推挽输出（PWM7）
    P0M1 &= ~0x04;
    P0M0 |=  0x04;
}

void UART_config(void) {
    COMx_InitDefine		COMx_InitStructure;

    // UART1（调试）
    COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;
    COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;
    COMx_InitStructure.UART_BaudRate  = 115200ul;
    COMx_InitStructure.UART_RxEnable  = ENABLE;
    COMx_InitStructure.BaudRateDouble = DISABLE;
    UART_Configuration(UART1, &COMx_InitStructure);
    NVIC_UART1_Init(ENABLE,Priority_1);
    UART1_SW(UART1_SW_P30_P31);

    // UART2（蓝牙 KT6368A）
    COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;
    COMx_InitStructure.UART_BRT_Use   = BRT_Timer2;
    COMx_InitStructure.UART_BaudRate  = 115200ul;
    COMx_InitStructure.UART_RxEnable  = ENABLE;
    COMx_InitStructure.BaudRateDouble = DISABLE;
    UART_Configuration(UART2, &COMx_InitStructure);
    NVIC_UART2_Init(ENABLE,Priority_1);
    UART2_SW(UART2_SW_P10_P11);
}

void PWM_config(void) {
    PWMx_InitDefine PWMx_InitStructure;

    // PWMA / PWM4 -> P2.6
    PWMx_InitStructure.PWM_Period        = PERIOD - 1;
    PWMx_InitStructure.PWM_DeadTime      = 0;
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE;
    PWMx_InitStructure.PWM_CEN_Enable    = ENABLE;
    PWM_Configuration(PWMA, &PWMx_InitStructure);

    PWMx_InitStructure.PWM_Mode      = CCMRn_PWM_MODE1;
    PWMx_InitStructure.PWM_Duty      = dutyA.PWM4_Duty;
    PWMx_InitStructure.PWM_EnoSelect = ENO4P;
    PWM_Configuration(PWM4, &PWMx_InitStructure);

    PWMA_Prescaler(PREESCALER - 1);
    PWM4_USE_P26P27();
    PWMA_BKR   |= 0x80;
    PWMA_CCER2 |= 0x10;
    NVIC_PWM_Init(PWMA, DISABLE, Priority_0);

    // PWMB / PWM7 -> P0.2
    PWMx_InitStructure.PWM_Period        = PERIOD - 1;
    PWMx_InitStructure.PWM_DeadTime      = 0;
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE;
    PWMx_InitStructure.PWM_CEN_Enable    = ENABLE;
    PWM_Configuration(PWMB, &PWMx_InitStructure);

    PWMx_InitStructure.PWM_Mode      = CCMRn_PWM_MODE1;
    PWMx_InitStructure.PWM_Duty      = dutyB.PWM7_Duty;
    PWMx_InitStructure.PWM_EnoSelect = ENO7P;
    PWM_Configuration(PWM7, &PWMx_InitStructure);

    PWMB_Prescaler(PREESCALER - 1);
    PWM7_SW(PWM7_SW_P02);
    PWMB_BKR   |= 0x80;
    PWMB_CCER2 |= 0x10;
    NVIC_PWM_Init(PWMB, DISABLE, Priority_0);
}

void servo_update(void) {
    u16 dutyA_val;
    u16 dutyB_val;

    // P2.6 舵机：0~90° -> 0.5~1.5ms
    dutyA_val = 500 + (angleA * 1000 / 90.0f);
    dutyA.PWM4_Duty = PERIOD * dutyA_val / 20000;
    UpdatePwm(PWM4, &dutyA);

    // P0.2 舵机：0~180° -> 0.5~2.5ms
    dutyB_val = 500 + (angleB * 2000 / 180.0f);
    dutyB.PWM7_Duty = PERIOD * dutyB_val / 20000;
    UpdatePwm(PWM7, &dutyB);
}

void sys_init() {
    EAXSFR();
    GPIO_config();
    UART_config();

    dutyA.PWM4_Duty = PERIOD * 1.0f / 20;
    dutyB.PWM7_Duty = PERIOD * 1.5f / 20;
    PWM_config();
    servo_update();

    EA = 1;
}

void main_start() _task_ 0 {
    sys_init();
    os_create_task(1);
    os_create_task(2);
    os_delete_task(0);
}

/* ================= 任务1：蓝牙接收 ================= */
void on_uart2_recv() {
    u8 * buf = RX2_Buffer;

    if (buf[0] != 0xDD || buf[1] != 0x77) {
        return;
    }

    joy_x  = buf[2];
    flag_A = buf[4];
    flag_B = buf[5];
    flag_D = buf[7];
}

void task_1() _task_ 1 {
    while(1) {
        if(COM2.RX_TimeOut > 0 && --COM2.RX_TimeOut == 0) {
            if(COM2.RX_Cnt > 0) {
                on_uart2_recv();
            }
            COM2.RX_Cnt = 0;
        }
        os_wait2(K_TMO, 1);
    }
}

/* ================= 任务2：舵机控制 ================= */
void task_2() _task_ 2 {
    static u8 last_flag_D = 0;

    while(1) {
        /* ===== D 键：按下瞬间启动复位 ===== */
        if (flag_D && !last_flag_D) {
            flag_reset = 1;
        }
        last_flag_D = flag_D;

        /* ===== 复位过程：缓慢逼近初始角度 ===== */
        if (flag_reset) {
            u8 done = 1;

            if (angleA < ANGLE_A_INIT) {
                angleA += 0.5f;
                if (angleA > ANGLE_A_INIT) angleA = ANGLE_A_INIT;
                done = 0;
            } else if (angleA > ANGLE_A_INIT) {
                angleA -= 0.5f;
                if (angleA < ANGLE_A_INIT) angleA = ANGLE_A_INIT;
                done = 0;
            }

            target_B = ANGLE_B_INIT;
            if (angleB < ANGLE_B_INIT) {
                angleB += 0.5f;
                if (angleB > ANGLE_B_INIT) angleB = ANGLE_B_INIT;
                done = 0;
            } else if (angleB > ANGLE_B_INIT) {
                angleB -= 0.5f;
                if (angleB < ANGLE_B_INIT) angleB = ANGLE_B_INIT;
                done = 0;
            }

            servo_update();

            if (done) flag_reset = 0;

            os_wait2(K_TMO, 4);
            continue;
        }

        /* ===== P2.6 舵机：A/B 键 ===== */
        if (flag_A) {
            angleA += 0.5f;
            if (angleA > 90) angleA = 90;
            servo_update();
        } else if (flag_B) {
            angleA -= 0.5f;
            if (angleA < 0) angleA = 0;
            servo_update();
        }

        /* ===== P0.2 舵机：摇杆（方向已反转） ===== */
        if (joy_x > JOY_MID + JOY_DEAD) {
            target_B = 180 - (u16)(joy_x - JOY_MID) * 180 / (255 - JOY_MID);
            if (target_B < 0) target_B = 0;
        }

        if (angleB < target_B) {
            angleB += 1;
            if (angleB > target_B) angleB = target_B;
            servo_update();
        } else if (angleB > target_B) {
            angleB -= 1;
            if (angleB < target_B) angleB = target_B;
            servo_update();
        }

        os_wait2(K_TMO, 4); // 20ms
    }
}