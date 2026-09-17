#include 	"RTX51TNY.H"
#include	"GPIO.h"
#include 	"UART.h"
#include 	"NVIC.h"
#include 	"Switch.h"

#include "Light.h"
#include "Key.h"
#include "Battery.h"
#include "Buzzer.h"
#include "Ultrasonic.h"
#include "Motors.h"
#include "Track.h"

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
	
	// ============= 外设初始化
	Light_init();  	// 车灯
	Key_init();	   	// 独立按键
	Battery_init(); // 电池电压
	Buzzer_init();  // 蜂鸣器
	Ultrasonic_init(); // 超声波
	Motors_init();	// 电机
	Track_init();	// 巡线
	
	printf("==sys_init==\n");
}

#define  TEST_TASK_ID	1
#define  KEY_TASK_ID	2
#define  TRACK_TASK_ID	3
#define  UART1_TASK_ID	4
#define  UART2_TASK_ID	5


// 入口不是main   是任务0
void main_task() _task_  0 { 
	sys_init();
	
	// 创建任务
//	os_create_task(TEST_TASK_ID); // 灯测试
	os_create_task(KEY_TASK_ID);  // 独立按键
	
	os_create_task(UART1_TASK_ID); // 串口1   调试
	os_create_task(UART2_TASK_ID); // 串口2   蓝牙
	
	os_delete_task(0); // 删除任务本身，可以调度别的任务
}

void test_task() _task_  TEST_TASK_ID { 
	while(1) {
		Light_on(LEFT); // 左亮
		os_wait2(K_TMO, 100); // 100*5 = 500ms
		Light_off(LEFT); // 左灭
		os_wait2(K_TMO, 100); // 100*5 = 500ms
		Light_on(RIGHT); // 右亮
		os_wait2(K_TMO, 100); // 100*5 = 500ms
		Light_off(RIGHT); // 右灭
		os_wait2(K_TMO, 100); // 100*5 = 500ms
		Light_on(ALL);// 全部亮
		os_wait2(K_TMO, 100); // 100*5 = 500ms
		Light_off(ALL); // 全部灭
		os_wait2(K_TMO, 100); // 100*5 = 500ms
	}
}

// 按键的回调函数
void Key_on_keydown_bk() {
	float vol; // 电压
	char res;   // 状态码
	float distance;  // 距离
	printf("key down\n");
	
	vol = Battery_get_voltage();
	printf("battery = %.2f\n", vol);
	
//	Buzzer_demo_2tiger(); // 2只老虎
	Buzzer_alarm();  // 警告声
	res = Ultrasonic_get_distance(&distance);
	if (res == 0) { // 0说明成功
		printf("distance = %.2f cm\n", distance);
	} else { // 打印错误码，方便调试
		printf("res = %d\n", (int)res);
	}
	
}



u8 flag = 1;
char speed = 30;
// 按键的回调函数
void Key_on_keydown() {
//	printf("key down flag = %d\n", (int)flag);
	float vol; // 电压
	char res;   // 状态码
	float distance;  // 距离
	// printf("key down\n");
	
	vol = Battery_get_voltage();
	printf("battery = %.2f\n", vol);
	
//	Buzzer_demo_2tiger(); // 2只老虎
	Buzzer_alarm();  // 警告声
	res = Ultrasonic_get_distance(&distance);
	if (res == 0) { // 0说明成功
		printf("distance = %.2f cm\n", distance);
	} else { // 打印错误码，方便调试
		printf("res = %d\n", (int)res);
	}
	
	switch(flag){
		case 1: 
			printf("==开启巡线任务==\n");
			os_create_task(TRACK_TASK_ID);
			break;  
		case 2:
			printf("==删除巡线任务==\n");	
			os_delete_task(TRACK_TASK_ID); 
		
			Motors_stop(); // 删除任务不能停止电机，人为停止
		
			break; 
		default:  
			break; 
	}
	
	flag++;
	if (flag > 2) flag = 1;
}



void Key_on_keyup() {
	printf("key up\n");
}

void track_task() _task_  TRACK_TASK_ID { // 巡线
	int pos = 0;
	char speed = 25;  // 不要太快    < 18 车动不了，压差不够
	while(1) {
		pos = Track_get_position();
//		printf("pos = %d\n", pos);	
		if (pos < 0) {  // 左拐
			Motors_turn(speed, LEFT_M);
		} else if (pos == 0) { // 前进
			Motors_forward(speed, MID_M);
		} else if (pos > 0) { // 右拐
			Motors_turn(speed, RIGHT_M);
		}
		
	
		// 真正巡线时候，时间不能太长    15ms差不多了
		os_wait2(K_TMO, 3); // 5 * 3 = 15ms   如果调试，时间可以长一点
	}
}

void key_task() _task_  KEY_TASK_ID { // 独立按键扫描
	while(1) {
		// 扫描按键
		Key_scan2(Key_on_keydown, NULL);
		os_wait2(K_TMO, 2); // 5 * 2 = 10ms
	}
}

void uart1_recv_task() _task_  UART1_TASK_ID { // 串口1接受到的数据
	u8 i;
	while(1) {
		if(COM1.RX_TimeOut > 0) {
			//超时计数
			if(--COM1.RX_TimeOut == 0) {
				if(COM1.RX_Cnt > 0) {
					for(i=0; i<COM1.RX_Cnt; i++)	{
						// 串口1收到的数据放在 RX1_Buffer[i]  通过串口2发送 TX2_write2buff
						TX2_write2buff(RX1_Buffer[i]);
					}
				}
				COM1.RX_Cnt = 0;
			}
		}

		// 不要处理的太快
		os_wait2(K_TMO, 1);
	}
}

/*****************************************
 0	1   2  3  4  5  6  7  数组索引
 帧头   x  y  A  B  C  D
DD 77  EF 50 01 00 01 00  数据示例

帧头 01：DD 77
方向 23：EF 50
按钮 4567: ABCD  按下01 抬起00

A: 蜂鸣器/车灯
B: 左旋转: 按下开始转,抬起停止转
C: 右旋转: 按下开始转,抬起停止转
D: 开启/关闭巡线

1. A键: 蜂鸣器/车灯 (边缘触发：只在按下的瞬间执行一次)
2. D键: 开启/关闭巡线 (边缘触发)
3. 互斥锁：如果开启了巡线，屏蔽手动驾驶，直接退出
4. 运动控制 (无巡线时生效)
    B/C键: 旋转 (电平触发：按住持续生效)
    摇杆控制: 只有在没有按下旋转按键时，摇杆才生效
*****************************************/
// value不能是char类型，数据会溢出
// 限制速度值，只能在 ~100 - 100 区间
static char LimitSpeed(int value){ 
	if (value > 100) return 100;
	else if (value < -100) return -100;
	
	return value;
}


void do_work_app() { // RX2_Buffer 全局数组，蓝牙接收到的数据，放在这
	u8 * buf = RX2_Buffer;
//	u8 i;
	char x, y;
	
	// static变量，函数调用完毕不释放
	static u8 led_flag = 0;     // 1:灯亮, 0:灯灭
	static u8 is_tracking = 0;  // 1:巡线开启, 0:关闭
	static u8 is_turning = 0; 	// 1:正在原地旋转, 0:未旋转
	
	// 提取当前按键状态
	u8 cur_A = buf[4];
	u8 cur_B = buf[5];
	u8 cur_C = buf[6];
	u8 cur_D = buf[7];
	
//	printf("===================================\n");
//	for(i=0; i < 8; i++) {
//		printf("0x%02X ", (int)buf[i]);
//	}
//	printf("\n===================================\n");
	
	// 处理帧头
	if (buf[0] != 0xDD || buf[1] != 0x77) {
		printf("帧头不对\n");
		return;
	}
	
//	1. A键: 蜂鸣器/车灯 (边缘触发：只在按下的瞬间执行一次)
	if (cur_A) { // 只要按下了，为1，非0就是真
//		printf("蜂鸣器\n");
		Buzzer_alarm();  // 警告声
		if (led_flag == 0) { // 灯是灭的，需要亮
//			printf("灯亮\n");
			Light_on(ALL);// 全部亮
		} else { // 灯是亮的，需要灭
//			printf("灯灭\n");
			Light_off(ALL);// 全部灭
		}
		// 标志位状态翻转
		led_flag = !led_flag;
	}
	
	
//	2. D键: 开启/关闭巡线 (边缘触发)
	if (cur_D) { // 只要按下了，为1，非0就是真
		if (is_tracking == 0) { // 没有巡线，需要开启巡线
//			printf("开启巡线任务\n");
			os_create_task(TRACK_TASK_ID);
			
		} else { // 有巡线，关闭
//			printf("删除巡线任务\n");
			os_delete_task(TRACK_TASK_ID); 
		
			Motors_stop(); // 删除任务不能停止电机，人为停止
		}
		is_tracking = !is_tracking;
	}
	
//	3. 互斥锁：如果开启了巡线，屏蔽手动驾驶，直接退出
	if (is_tracking) return;
	
	
//	4. 运动控制 (无巡线时生效)
//		B/C键: 旋转 (电平触发：按住持续生效)
	// B: 左旋转: 按下开始转,抬起停止转
	if (cur_B) { // 按下B
		if (is_turning == 0) {
//			printf("左旋转\n");
			Motors_around(30, LEFT_M);
			is_turning = 1;
		}
	} else if (cur_C) { // 按下C
		if (is_turning == 0) {
//			printf("右旋转\n");
			Motors_around(30, RIGHT_M);
			is_turning = 1;
		}
	} else { // B和C抬起
		if (is_turning == 1) {
			printf("停止旋转\n");
			
			is_turning = 0;
		}
	}
	
	if (is_turning == 1) return;  // 旋转时候，摇杆不能工作
	
//    摇杆控制: 只有在没有按下旋转按键时，摇杆才生效
	x = buf[2], y = buf[3];
//	printf("(x, y) = (%d, %d)\n", (int)x, (int)y);
//	printf("左前=%d, 左后=%d, 右前=%d, 右后=%d\n", (int)(x+y), (int)(y-x), (int)(y-x), (int)(x+y));
//	printf("[限制]左前=%d, 左后=%d, 右前=%d, 右后=%d\n", (int)LimitSpeed(x+y), (int)LimitSpeed(y-x), (int)LimitSpeed(y-x), (int)LimitSpeed(x+y));
	Motors_move(x, y);
}

void uart2_recv_task() _task_  UART2_TASK_ID { // 串口2接收到的数据，串口2就是蓝牙
	u8 i;
	while(1) {
		if(COM2.RX_TimeOut > 0) {
			//超时计数
			if(--COM2.RX_TimeOut == 0) {
				if(COM2.RX_Cnt > 0) {
					
					// 蓝牙数据的处理
					do_work_app();
					
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



