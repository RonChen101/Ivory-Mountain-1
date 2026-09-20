#include "App.h"
#include "App_Track.h"
#include "App_Vehicle.h"
#include "TrackSensor.h"
#include "Motors.h"

/*
App_Track
巡线控制：读传感器位图 -> 计算偏移 -> 差速转向
算法与原 App_Motor.c 完全一致，仅传感器读取改为 TrackSensor 驱动
*/

// 初始化
void Track_init(void) {
	TrackSensor_init();
}

// 计算偏移坐标：只有中间3路参与计算（左右边缘2路保留在位图中备用）
int Track_get_position() {
	static int last_pos = 0; // 上一次状态， static 类型
	int pos = 0;   // 当前的坐标
	u8 cnt = 0;    // 标识几个灯压到黑线了
	u8 bitmap = TrackSensor_read();

	if (bitmap & TS_LEFT2) {
		pos += -32;
		cnt++;
	}
	if (bitmap & TS_MID) {
		// 中间权重 0
		cnt++;
	}
	if (bitmap & TS_RIGHT1) {
		pos += 32;
		cnt++;
	}
	if (cnt == 0) { // 没有压到黑线，返回上一次状态
		return last_pos;
	}
	// 当前状态的平均值
	pos = pos / cnt;
	// 更新上一次状态
	last_pos = pos;

	return pos;
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
