#include "Track.h"

// 初始化
void Track_init(){
	//准双向口	
	P0_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4);
}


// 获取寻迹坐标： 高电平-不亮-压到黑线  低电平-亮起-正常反射面
int Track_get_position() {
	static int last_pos = 0; // 上一次状态， static 类型
	int pos = 0;   // 当前的坐标
	u8 cnt = 0;    // 标识几个灯压到黑线了
//	if (LED1 == 1) {
//		pos += -64;
//		cnt++;
//	}
	if (LED2 == 1) {
		pos += -32;
		cnt++;
	}
	if (LED3 == 1) {
		pos += 0;
		cnt++;
	}
	if (LED4 == 1) {
		pos += 32;
		cnt++;
	}
//	if (LED5 == 1) {
//		pos += 64;
//		cnt++;
//	}
	if (cnt == 0) { // 没有压到黑线，返回上一次状态
		return last_pos;
	}
	// 当前状态的平均值
	pos = pos / cnt;
	// 更新上一次状态
	last_pos = pos;

	return pos;
}