#include "Motors.h"
#include "App.h"
/* 车辆状态机：唯一能"指挥"电机的地方
   - 巡线任务的创建/删除只在这里发生（单一状态源）
   - 电机命令只有一个写者（RTX 任务 track_task + 下面的手动接口），
     从架构上消除 C51 非重入函数被多任务并发调用的覆盖层问题 */

static VehicleMode mode = VEH_IDLE;
static u8 is_turning = 0;	// 1:正在原地旋转, 0:未旋转

void Vehicle_set_mode(VehicleMode m) {
	if (m == mode) return; // 幂等

	if (m == VEH_TRACKING) {
		// 开启巡线任务
		printf("[dbg]1 before create\n");	// TODO: 临时调试探针，定位复位点后删除
		os_create_task(TRACK_TASK_ID);
		printf("[dbg]2 after create\n");	// TODO: 临时调试探针
	} else if (mode == VEH_TRACKING) {
		// 从巡线退出：删除任务并停车（删除任务不会停电机，人为停止）
		os_delete_task(TRACK_TASK_ID);
		Motors_stop();
	}
	mode = m;
}

VehicleMode Vehicle_get_mode(void) {
	return mode;
}

void Vehicle_rotate(char speed, MotorsMode dir) {
	if (mode == VEH_TRACKING) return;	// 巡线互斥：屏蔽手动驾驶
	if (is_turning) return;				// 已在旋转，不重复发命令

	Motors_around(speed, dir);
	is_turning = 1;
}

u8 Vehicle_is_rotating(void) {
	return is_turning;
}

void Vehicle_stop_rotate(void) {
	if (!is_turning) return;

	is_turning = 0;
	Motors_stop();	// 停转兜底；随后同一帧的摇杆数据会立即接管
}

void Vehicle_manual_move(char x, char y) {
	if (mode == VEH_TRACKING) return;	// 巡线互斥
	if (is_turning) return;				// 旋转时摇杆不生效

	Motors_move(x, y);
}
