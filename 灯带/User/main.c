#include 	"RTX51TNY.H"
#include	"GPIO.h"
#include 	"UART.h"
#include 	"NVIC.h"
#include 	"Switch.h"

#include "WS2812.h"

void GPIO_config(void) {
	GPIO_InitTypeDef	GPIO_InitStructure;		//结构定义
	// UART1 P30 P31 准双向
	GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1;		//指定要初始化的IO,
	GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P3, &GPIO_InitStructure);//初始化
}

void UART_config(void) {
	// >>> 记得添加 NVIC.c, UART.c, UART_Isr.c <<<
    COMx_InitDefine		COMx_InitStructure;					//结构定义
    COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;	//模式, UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
    COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;			//选择波特率发生器, BRT_Timer1, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
    COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率, 一般 110 ~ 115200
    COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
    COMx_InitStructure.BaudRateDouble = DISABLE;			//波特率加倍, ENABLE或DISABLE
    UART_Configuration(UART1, &COMx_InitStructure);		//初始化串口1 UART1,UART2,UART3,UART4

  	NVIC_UART1_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
    UART1_SW(UART1_SW_P30_P31);		// 引脚选择, UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44
}

void sys_init() {
	EA = 1;			// 使能全局中断
	EAXSFR();		/* 扩展寄存器访问使能 */
	
	GPIO_config(); // IO配置
	UART_config(); // 串口配置
	
	// ============= 外设初始化
	
	printf("==sys_init==\n");
}

#define  TEST_TASK_ID	1

// 入口不是main   是任务0
void main_task() _task_  0 { 
	sys_init();
	
	// 创建任务
	os_create_task(TEST_TASK_ID); // 测试
	
	os_delete_task(0); // 删除任务本身，可以调度别的任务
}

void test_task() _task_  TEST_TASK_ID { 
    // 初始化并设置全部灯为红色，调用一次即可，灯带会自己锁存保持
    WS2812_Init_And_Set(0, 255, 0); // 紫
//    WS2812_Init_And_Set(0, 0, 255);     //黄色
//    WS2812_Init_And_Set(255, 0, 0);//浅蓝色
//    WS2812_Init_And_Set(255, 0, 255); //绿
//    WS2812_Init_And_Set(255, 255, 0); //深蓝色
//    WS2812_Init_And_Set(0, 255, 255);//红色
//    WS2812_Init_And_Set(255, 255, 255);// 无颜色只有第一个亮绿

    // 固定颜色不需要反复刷新，空转让出 CPU 给其他任务
    while(1) {
        os_wait(K_TMO, 100, 0);   // 每 100 个 tick 醒一次，什么都不做
    }
}