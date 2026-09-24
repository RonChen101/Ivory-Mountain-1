#include 	"RTX51TNY.H"
#include	"GPIO.h"
#include 	"UART.h"
#include 	"NVIC.h"
#include 	"Switch.h"

void GPIO_config(void) {
	GPIO_InitTypeDef	GPIO_InitStructure;		//结构定义
	// ============= UART1 P30 P31 准双向
	GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1;		//指定要初始化的IO,
	GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P3, &GPIO_InitStructure);//初始化
	
	// ============= UART2 P10 P11 准双向
	GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1;		//指定要初始化的IO,
	GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P1, &GPIO_InitStructure);//初始化
}

void UART_config(void) {
	// >>> 记得添加 NVIC.c, UART.c, UART_Isr.c <<<
    COMx_InitDefine		COMx_InitStructure;					//结构定义
	// ======================== UART1 
    COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;	//模式, UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
    COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;			//选择波特率发生器, BRT_Timer1, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
    COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率, 一般 110 ~ 115200
    COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
    COMx_InitStructure.BaudRateDouble = DISABLE;			//波特率加倍, ENABLE或DISABLE
    UART_Configuration(UART1, &COMx_InitStructure);		//初始化串口1 UART1,UART2,UART3,UART4

  	NVIC_UART1_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
    UART1_SW(UART1_SW_P30_P31);		// 引脚选择, UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44


	// ======================== UART2 
    COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;	//模式, UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
    COMx_InitStructure.UART_BRT_Use   = BRT_Timer2;			//选择波特率发生器, BRT_Timer1, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
    COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率, 一般 110 ~ 115200
    COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
    COMx_InitStructure.BaudRateDouble = DISABLE;			//波特率加倍, ENABLE或DISABLE
    UART_Configuration(UART2, &COMx_InitStructure);		//初始化串口1 UART1,UART2,UART3,UART4

  	NVIC_UART2_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
    UART2_SW(UART2_SW_P10_P11);		// 引脚选择
}

void sys_init() {
	EA = 1;			// 使能全局中断
	EAXSFR();		/* 扩展寄存器访问使能 */
	
	GPIO_config(); // IO配置
	UART_config(); // 串口配置
	
	printf("==sys_init==\n");
}

#define UART1_TASK_ID   0
#define UART2_TASK_ID   1

// 入口不是main   是任务0
void main_task() _task_  0 { 
	sys_init();
	
	os_create_task(UART1_TASK_ID); // 串口1   调试
	os_create_task(UART2_TASK_ID); // 串口2   蓝牙

	
	os_delete_task(0); // 删除任务本身，可以调度别的任务
}

void uart2_recv_task() _task_  UART2_TASK_ID { // 串口2接收到的数据，串口2就是蓝牙
	u8 i;
	while(1) {
		if(COM2.RX_TimeOut > 0) {
			//超时计数
			if(--COM2.RX_TimeOut == 0) {
				if(COM2.RX_Cnt > 0) {
					
					// 蓝牙数据的处理
					//do_work_app();
					
					for(i=0; i<COM2.RX_Cnt; i++)	{
						// 串口2收到的数据放在 RX2_Buffer[i]  通过串口1发送 TX1_write2buff
						TX1_write2buff(RX2_Buffer[i]);
					}
				}
				COM2.RX_Cnt = 0;
			}
		}
		
		// 不要处理的太快
		os_wait2(K_TMO, 1);
	}
}