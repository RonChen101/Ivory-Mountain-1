#include "Light.h"

// 初始化
void Light_init() {
	// IO 初始化   准双向  或 推挽输出
	// ========== 准双向
	P0_MODE_IO_PU(GPIO_Pin_7); // P07
	P5_MODE_IO_PU(GPIO_Pin_2); // P52
	
//	// ========== 推挽输出
//	P0_MODE_OUT_PP(GPIO_Pin_7);
//	P5_MODE_OUT_PP(GPIO_Pin_2);
}


// 开灯
void Light_on(Light temp) {
	if (temp == LEFT) { // 左
		LED_LEFT = 1;
	} else if (temp == RIGHT) { // 右
		LED_RIGHT = 1;
	} else { // 全部
		LED_LEFT = 1;
		LED_RIGHT = 1;
	}
}


// 关灯
void Light_off(Light temp) {
	if (temp == LEFT) { // 左
		LED_LEFT = 0;
	} else if (temp == RIGHT) { // 右
		LED_RIGHT = 0;
	} else { // 全部
		LED_LEFT = 0;
		LED_RIGHT = 0;
	}
}