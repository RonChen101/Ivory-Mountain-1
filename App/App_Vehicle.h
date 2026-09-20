#ifndef __APP_VEHICLE_H__
#define __APP_VEHICLE_H__

#include "App.h"
#include "Motors.h"

/* 车辆模式：车辆状态机的唯一状态源
   蓝牙、按键都通过 Vehicle_set_mode 切换，不允许各自直接
   os_create_task / os_delete_task 巡线任务 */
typedef enum {
	VEH_IDLE,		// 上电初始
	VEH_MANUAL,		// 蓝牙手动驾驶
	VEH_TRACKING	// 巡线中
} VehicleMode;

// 切换车辆模式（幂等：重复设置同一模式不会重复创建/删除任务）
void Vehicle_set_mode(VehicleMode m);

VehicleMode Vehicle_get_mode(void);

// 原地旋转（手动模式生效，巡线时被屏蔽）
void Vehicle_rotate(char speed, MotorsMode dir);
u8   Vehicle_is_rotating(void);
void Vehicle_stop_rotate(void);

// 摇杆移动（手动模式且非旋转时生效）
void Vehicle_manual_move(char x, char y);

#endif
