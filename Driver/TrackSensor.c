#include "TrackSensor.h"

/* 传感器引脚 */
#define LED1 P00	// 左1
#define LED2 P01	// 左2
#define LED3 P02	// 中
#define LED4 P03	// 右1
#define LED5 P04	// 右2

// 初始化
void TrackSensor_init(void) {
	//准双向口
	P0_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4);
}

// 读取5路巡迹传感器，返回位图 TS_xxx（置位 = 压到黑线）
u8 TrackSensor_read(void) {
	u8 bitmap = 0;

	if (LED1) bitmap |= TS_LEFT1;
	if (LED2) bitmap |= TS_LEFT2;
	if (LED3) bitmap |= TS_MID;
	if (LED4) bitmap |= TS_RIGHT1;
	if (LED5) bitmap |= TS_RIGHT2;

	return bitmap;
}
