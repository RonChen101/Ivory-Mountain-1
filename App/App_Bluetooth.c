#include "UART.h"
#include "App.h"
#include "Motors.h" 
#include "Buzzer.h"
#include "Light.h"

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