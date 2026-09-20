#include "config.h"
#include "App.h"



void main_task() _task_  0 { 
	sys_init();
	
	// 创建任务
	os_create_task(KEY_TASK_ID);  // 独立按键
	
	os_create_task(UART1_TASK_ID); // 串口1   调试
	os_create_task(UART2_TASK_ID); // 串口2   蓝牙
	
	os_delete_task(0); // 删除任务本身，可以调度别的任务
}









