#include "App.h"
#include "App_Vehicle.h"
#include "Key.h"

// 按键的回调函数：切换巡线开/关
// 状态全部由 App_Vehicle 管理，
void Key_on_keydown() {
	if (Vehicle_get_mode() == VEH_TRACKING) {
		printf("==关闭巡线任务==\n");
		Vehicle_set_mode(VEH_MANUAL);
	} else {
		printf("==开启巡线任务==\n");
		Vehicle_set_mode(VEH_TRACKING);
	}
}

void Key_on_keyup() {
	printf("key up\n");
}

void key_task() _task_  KEY_TASK_ID { // 独立按键扫描
	while(1) {
		// 扫描按键
		Key_scan();
		os_wait2(K_TMO, 2); // 5 * 2 = 10ms
	}
}
