#include	"GPIO.h"
#include	"Delay.h"
#include	"NVIC.h"
#include 	"UART.h"
#include 	"Switch.h"
#include 	"Nixie.h"

// 宏定义，给引脚起别名
#define  LED_SW     P45
#define  LED1       P27
#define  LED2       P26
#define  LED3       P15
#define  LED4       P14
#define  LED5       P23
#define  LED6       P22
#define  LED7       P21
#define  LED8       P20

void GPIO_config() { 
	// 串口1 P30 P31  准双向口
	P3_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1);
	
	// LED1~LED8 灯组 准双向
	// P45
	P4_MODE_IO_PU(GPIO_Pin_5);
	// P20 P21 P22 P23 P26 P27
	P2_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7);
	// P14 P15
	P1_MODE_IO_PU(GPIO_Pin_4 | GPIO_Pin_5);

	// 三极管为低，导通
	LED_SW = 0;
	
	// P01 震动马达 推挽，电流大一些
	P0_MODE_OUT_PP(GPIO_Pin_1);
	P01 = 0; // 马达默认不震动
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

// 串口1接收逻辑处理
void deal_uart1_recv() {
	u8 *buf = RX1_Buffer; // 原来的数组名太长了，换个名保存一下
	char i;
	
	// 帧头(2) | 命令(1) | 数据长度(1) | 数据(n) | 校验位(1) | 帧尾(1)
	//          0  1 2  3  4   5 6
	// 串口收到 AA AA 00 01 01 00 BB，LED灯组亮灭1次
	// 串口收到 AA AA 00 01 02 03 BB,   数码管走马灯 1~8
	// 串口收到 AA AA 00 01 03 02 BB，马达震动一会后停止
	
	// 帧头不对，就不要往下执行
	if (buf[0] != 0xAA || buf[1] != 0xAA) return;
	// 命令不对，就不要往下执行
	if (buf[2] != 0x00) return;
	// 数据长度不对，就不要往下执行
	if (buf[3] != 0x01) return;
	// 帧尾不对, 就不要往下执行
	if (buf[6] != 0xBB) return;
	
	// 数据 和 校验值  来确定操作啥
	if (buf[4] == 0x01 && buf[5] == 0x00) { // LED灯组亮灭1次
		LED8 = LED7 = LED6 = LED5 = LED4 = LED3 = LED2 = LED1 = 0; // 低电平亮
		delay_ms(250);
//		delay_ms(250);
		LED8 = LED7 = LED6 = LED5 = LED4 = LED3 = LED2 = LED1 = 1; // 高电平灭
		delay_ms(250);
//		delay_ms(250);
	
	} else if (buf[4] == 0x02 && buf[5] == 0x03) { // 数码管走马灯 1~8
		for (i = 1; i <= 8; i++) { // i = 1, 2, …… 8
			//          内容, 第几个数码
			Nixie_display(i, i);
			delay_ms(250);	
		}
		Nixie_clear(); // 灯灭
	
	} else if (buf[4] == 0x03 && buf[5] == 0x02) { // 马达震动一会后停止
		P01 = 1; // 震动
		delay_ms(250);
		delay_ms(250);
//		delay_ms(250);
//		delay_ms(250);
		P01 = 0; // 停止
	}
}

void main() { // 入口
//	u8 i;
	EA = 1;
	
	GPIO_config(); // IO配置
	UART_config(); // 串口
	Nixie_init();  // 数码管
	
	while(1) {
		if(COM1.RX_TimeOut > 0) {
			//超时计数
			if(--COM1.RX_TimeOut == 0) {
				if(COM1.RX_Cnt > 0) { // 接收到数据后，才能进来
					// 串口1接收逻辑处理
					deal_uart1_recv();
					
//					// for只能为了方便遍历多个数据，有没有for，不影响数据内容
//					for(i=0; i<COM1.RX_Cnt; i++)	{
//						// RX1_Buffer[i]存的是接收的每个字节，写出用 TX1_write2buff
//						TX1_write2buff(RX1_Buffer[i]); // 数据回显，可以不用
//					}
				}
				COM1.RX_Cnt = 0;
			}
		}

		// 不要处理的太快
		delay_ms(1);
	}
}