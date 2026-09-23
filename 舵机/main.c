#include "Config.h"
#include "Delay.h"
#include "STC8H_PWM.h"
#include "UART.h"
#include "NVIC.h"
#include "Switch.h"
#include "GPIO.h"

/* ================= 按键定义 ================= */
#define KEY1    P51
#define KEY2    P52
#define KEY3    P53
#define KEY4    P54

#define DOWN    0
#define UP      1

u8 last_state1 = UP;
u8 last_state2 = UP;
u8 last_state3 = UP;
u8 last_state4 = UP;

/* ================= PWM 相关 ================= */
#define PREESCALER  10
#define FREQ        50
#define PERIOD (MAIN_Fosc / FREQ / PREESCALER)

PWMx_Duty dutyB = {0};   // PWMB，PWM7 -> P0.2
PWMx_Duty dutyA = {0};   // PWMA，PWM4 -> P2.6

/* 当前角度 */
float angleB = 90;   // P0.2 舵机，0~180°
float angleA = 45;   // P2.6 舵机，0~90°

/* ================= GPIO 配置 ================= */
void GPIO_config(void)
{
    GPIO_InitTypeDef info;

    // UART1  P3.0  P3.1  准双向
    info.Mode = GPIO_PullUp;
    info.Pin  = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Inilize(GPIO_P3, &info);

    // 按键 P5.1 ~ P5.4 准双向
    P5_MODE_IO_PU(GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4);

    // P0.2 推挽输出（PWM7）
    P0M1 &= ~0x04;
    P0M0 |=  0x04;

    // P2.6 推挽输出（PWM4）
    P2M1 &= ~0x40;
    P2M0 |=  0x40;
}

/* ================= 串口配置 ================= */
void UART_config(void)
{
    COMx_InitDefine COMx_InitStructure;
    COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;
    COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;
    COMx_InitStructure.UART_BaudRate  = 115200ul;
    COMx_InitStructure.UART_RxEnable  = ENABLE;
    COMx_InitStructure.BaudRateDouble = DISABLE;
    UART_Configuration(UART1, &COMx_InitStructure);

    NVIC_UART1_Init(ENABLE, Priority_1);
    UART1_SW(UART1_SW_P30_P31);
}

/* ================= PWM 初始化 ================= */
void PWM_config(void)
{
    PWMx_InitDefine PWMx_InitStructure;

    // 先配置 PWMB 通用寄存器
    PWMx_InitStructure.PWM_Period        = PERIOD - 1;
    PWMx_InitStructure.PWM_DeadTime      = 0;
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE;
    PWMx_InitStructure.PWM_CEN_Enable    = ENABLE;
    PWM_Configuration(PWMB, &PWMx_InitStructure);

    // 再配置 PWM7 通道
    PWMx_InitStructure.PWM_Mode      = CCMRn_PWM_MODE1;
    PWMx_InitStructure.PWM_Duty      = dutyB.PWM7_Duty;
    PWMx_InitStructure.PWM_EnoSelect = ENO7P;
    PWM_Configuration(PWM7, &PWMx_InitStructure);

    // 先配置 PWMA 通用寄存器
    PWMx_InitStructure.PWM_Period        = PERIOD - 1;
    PWMx_InitStructure.PWM_DeadTime      = 0;
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE;
    PWMx_InitStructure.PWM_CEN_Enable    = ENABLE;
    PWM_Configuration(PWMA, &PWMx_InitStructure);

    // 再配置 PWM4 通道
    PWMx_InitStructure.PWM_Mode      = CCMRn_PWM_MODE1;
    PWMx_InitStructure.PWM_Duty      = dutyA.PWM4_Duty;
    PWMx_InitStructure.PWM_EnoSelect = ENO4P;
    PWM_Configuration(PWM4, &PWMx_InitStructure);

    // 预分频
    PWMB_Prescaler(PREESCALER - 1);
    PWMA_Prescaler(PREESCALER - 1);

    // 切换引脚
    PWM7_SW(PWM7_SW_P02);     // PWM7 -> P0.2
    PWM4_USE_P26P27();        // PWM4 -> P2.6

    // 双重保险
    PWMA_BKR   |= 0x80;   // MOE = 1
    PWMA_CCER2 |= 0x10;   // CC4E = 1

    NVIC_PWM_Init(PWMB, DISABLE, Priority_0);
    NVIC_PWM_Init(PWMA, DISABLE, Priority_0);
}

/* ================= 舵机角度输出 ================= */
void servo_update(void)
{
    u16 dutyB_val;
    u16 dutyA_val;

    // P0.2 舵机：0~180° -> 0.5~2.5ms
    dutyB_val = 500 + (angleB * 2000 / 180.0f);
    dutyB.PWM7_Duty = PERIOD * dutyB_val / 20000;
    UpdatePwm(PWM7, &dutyB);

    // P2.6 舵机：0~90° -> 0.5~1.5ms
    dutyA_val = 500 + (angleA * 1000 / 90.0f);
    dutyA.PWM4_Duty = PERIOD * dutyA_val / 20000;
    UpdatePwm(PWM4, &dutyA);
}

/* ================= 主函数 ================= */
int main(void)
{
    EAXSFR();
    GPIO_config();
    UART_config();

    dutyB.PWM7_Duty = PERIOD * 1.5f / 20;
    dutyA.PWM4_Duty = PERIOD * 1.0f / 20;

    PWM_config();
    servo_update();     // 上电先输出初始角度

    EA = 1;

    while(1)
    {
        /* ---------- KEY1：P0.2 舵机左转 ---------- */
        if(KEY1 == DOWN)
        {
            angleB -= 1;                     // 左转，角度减小
            if(angleB < 0) angleB = 0;
            servo_update();
        }

        /* ---------- KEY2：P0.2 舵机右转 ---------- */
        if(KEY2 == DOWN)
        {
            angleB += 1;                     // 右转，角度增大
            if(angleB > 180) angleB = 180;
            servo_update();
        }

        /* ---------- KEY3：P2.6 舵机左转 ---------- */
        if(KEY3 == DOWN)
        {
            angleA -= 0.5;                   // 左转
            if(angleA < 0) angleA = 0;
            servo_update();
        }

        /* ---------- KEY4：P2.6 舵机右转 ---------- */
        if(KEY4 == DOWN)
        {
            angleA += 0.5;                   // 右转
            if(angleA > 90) angleA = 90;
            servo_update();
        }

        delay_ms(10);    // 控制转动速度，数值越小转得越快
    }
}