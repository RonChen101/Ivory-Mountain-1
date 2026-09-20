#ifndef __TRACK_SENSOR_H__
#define __TRACK_SENSOR_H__

#include "GPIO.h"
#include "Type_def.h"

/* 巡迹传感器位图定义（置位 = 压到黑线）
   高电平-不亮-压到黑线  低电平-亮起-正常反射面 */
#define TS_LEFT1	0x01	// P00 左1 (-64)
#define TS_LEFT2	0x02	// P01 左2 (-32)
#define TS_MID		0x04	// P02 中  (0)
#define TS_RIGHT1	0x08	// P03 右1 (+32)
#define TS_RIGHT2	0x10	// P04 右2 (+64)

// 初始化（准双向口）
void TrackSensor_init(void);

// 读取5路巡迹传感器，返回位图 TS_xxx
u8 TrackSensor_read(void);

#endif
