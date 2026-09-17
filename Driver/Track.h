#ifndef __TRACK_H__
#define __TRACK_H__

#include "GPIO.h"

#define LED1 P00 // 左1	-64
#define LED2 P01 // 左2	-32
#define LED3 P02 // 中	0
#define LED4 P03 // 右1	32
#define LED5 P04 // 右2	64

// 初始化
void Track_init();
// 获取寻迹坐标： 高电平-不亮-压到黑线  低电平-亮起-正常反射面
int Track_get_position();

#endif