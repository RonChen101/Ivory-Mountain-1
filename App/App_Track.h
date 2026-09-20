#ifndef __APP_TRACK_H__
#define __APP_TRACK_H__

#include "App.h"

// 初始化巡迹传感器
void Track_init(void);

// 获取寻迹坐标：左偏为负(-32)，右偏为正(+32)，居中为0
// 无传感器压线时返回上一次坐标
int Track_get_position(void);

// 巡线任务（RTX-51 任务，ID = TRACK_TASK_ID，由 App_Vehicle 创建/删除）
void track_task(void);

#endif
