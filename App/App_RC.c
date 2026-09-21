#include "UART.h"
#include "App.h"
#include "Buzzer.h"
#include "Light.h"


#if KEY_TEST 
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
#endif

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

本模块只负责"协议解析 -> 意图"，车辆状态全部由 App_Vehicle 管理
*****************************************/

// 解析一帧蓝牙数据（RX2_Buffer 全局数组，len = COM2.RX_Cnt）
void RC_process_frame(u8 * buf, u8 len) {
	char x, y;
	u8 cur_A = buf[4];
	u8 cur_B = buf[5];
	u8 cur_C = buf[6];
	u8 cur_D = buf[7];

	// static变量，函数调用完毕不释放（记录上一帧按键状态，用于边缘检测）
	static u8 prev_A = 0;
	static u8 prev_D = 0;
	static u8 led_flag = 0;     // 1:灯亮, 0:灯灭

	if (len < 8) {  // 半截帧不处理
		return;
	}

	// 处理帧头
	if (buf[0] != 0xDD || buf[1] != 0x77) {
		printf("frame header error\n");
		return;
	}

//	1. A键: 蜂鸣器/车灯 (边缘触发：只在按下的瞬间执行一次)
	if (cur_A && !prev_A) { // 上一帧没按下，这一帧按下
		Buzzer_alarm();  // 警告声
		if (led_flag == 0) { // 灯是灭的，需要亮
			Light_on(ALL);// 全部亮
		} else { // 灯是亮的，需要灭
			Light_off(ALL);// 全部灭
		}
		// 标志位状态翻转
		led_flag = !led_flag;
	}
	prev_A = cur_A;

//	2. D键: 开启/关闭巡线 (边缘触发)
	if (cur_D && !prev_D) {
		if (Vehicle_get_mode() == VEH_TRACKING) { // 巡线中，关闭
			Vehicle_set_mode(VEH_MANUAL);
		} else { // 没有巡线，开启巡线
			Vehicle_set_mode(VEH_TRACKING);
		}
	}
	prev_D = cur_D;

//	3. 互斥锁：如果开启了巡线，屏蔽手动驾驶，直接退出
	if (Vehicle_get_mode() == VEH_TRACKING) return;


//	4. 运动控制 (无巡线时生效)
//		B/C键: 旋转 (电平触发：按住持续生效)
	if (cur_B) { // 按下B
		Vehicle_rotate(30, LEFT_M);
	} else if (cur_C) { // 按下C
		Vehicle_rotate(30, RIGHT_M);
	} else { // B和C抬起
		Vehicle_stop_rotate();
	}

	if (Vehicle_is_rotating()) return;  // 旋转时候，摇杆不能工作

//    摇杆控制: 只有在没有按下旋转按键时，摇杆才生效
	x = buf[2], y = buf[3];
	Vehicle_manual_move(x, y);
}

void uart2_recv_task() _task_  UART2_TASK_ID { // 串口2接收到的数据，串口2就是蓝牙
	u8 i;
	while(1) {
		if(COM2.RX_TimeOut > 0) {
			//超时计数
			if(--COM2.RX_TimeOut == 0) {
				if(COM2.RX_Cnt > 0) {

					// 蓝牙数据的处理
					// RC_process_frame(RX2_Buffer, COM2.RX_Cnt);

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
