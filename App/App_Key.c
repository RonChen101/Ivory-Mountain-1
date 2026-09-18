#include "App.h"
#include "Key.h"



u8 flag = 1;
char speed = 30;
// 按键的回调函数
void Key_on_keydown() {
//	printf("key down flag = %d\n", (int)flag);
	float vol; // 电压
	char res;   // 状态码
	float distance;  // 距离
	// printf("key down\n");
	
	vol = Battery_get_voltage();
	printf("battery = %.2f\n", vol);
	
//	Buzzer_demo_2tiger(); // 2只老虎
	Buzzer_alarm();  // 警告声
	res = Ultrasonic_get_distance(&distance);
	if (res == 0) { // 0说明成功
		printf("distance = %.2f cm\n", distance);
	} else { // 打印错误码，方便调试
		printf("res = %d\n", (int)res);
	}
	
	switch(flag){
		case 1: 
			printf("==开启巡线任务==\n");
			os_create_task(TRACK_TASK_ID);
			break;  
		case 2:
			printf("==删除巡线任务==\n");	
			os_delete_task(TRACK_TASK_ID); 
		
			Motors_stop(); // 删除任务不能停止电机，人为停止
		
			break; 
		default:  
			break; 
	}
	
	flag++;
	if (flag > 2) flag = 1;
}



void Key_on_keyup() {
	printf("key up\n");
}



void key_task() _task_  KEY_TASK_ID { // 独立按键扫描
	while(1) {
		// 扫描按键
		Key_scan2(Key_on_keydown, NULL);
		os_wait2(K_TMO, 2); // 5 * 2 = 10ms
	}
}
