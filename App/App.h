#ifndef __APP_H__
#define __APP_H__

#include "Config.h"
#include "STDIO.H"
#include "Motors.h"

//是否启动按键测试
#define KEY_TEST 1

//每个task对应ID
#define KEY_TASK_ID 1
#define TRACK_TASK_ID  4
#define UART1_TASK_ID  2
#define UART2_TASK_ID  3
#define BUZZER_TASK_ID 5

void sys_init(void);


// ----------------------------------循迹模块---------------------------------------
// 初始化巡迹传感器
void Track_init(void);

// 获取寻迹坐标：5路加权平均，左偏为负(最大-64)，右偏为正(最大+64)，居中为0
// 无有效压线时返回上一次坐标；坏灯已自动屏蔽
int Track_get_position(void);


//小车状态机
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
