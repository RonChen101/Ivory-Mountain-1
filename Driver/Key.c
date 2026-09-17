#include "Key.h"

// 初始化
void Key_init() {
//	//准双向口	P05
//	P0_MODE_IO_PU(GPIO_Pin_5);
	
	// 漏极开路	P05
	P0_MODE_OUT_OD(GPIO_Pin_5);

}

#define DOWN	0
#define	UP		1
static u8 last_state = UP; // 抬起
// 扫描按键
void Key_scan() {
#if 0
	// 上一次抬起，当前按下，按下才有效
	if (last_state == UP && KEY == DOWN) {
		last_state = DOWN; // 保存状态
		
		#if USE_KEYDOWN
		Key_on_keydown(); // 调用
		#endif
	
	// 上一次按下，当前抬起，抬起才有效
	} else if (last_state == DOWN && KEY == UP) {
		last_state = UP; // 保存状态
		#if USE_KEYUP
		Key_on_keyup(); // 调用
		#endif 
	}
#else
	// 上一次和当前不相等
	if (last_state != KEY) {
		last_state = KEY; // 保存状态
		// 判断当前状态
		if (KEY == DOWN) { // 按下
			#if USE_KEYDOWN
			Key_on_keydown(); // 调用
			#endif
		} else { // 抬起
			#if USE_KEYUP
			Key_on_keyup(); // 调用
			#endif 
		}
	}
#endif
}

// 扫描按键，函数指针回调函数版本
// 不要和Key_scan()同时使用
void Key_scan2(void (*down)(), void (*up)()) {
	// 上一次和当前不相等
	if (last_state != KEY) {
		last_state = KEY; // 保存状态
		// 判断当前状态
		if (KEY == DOWN) { // 按下

			if (down != NULL) down(); // 调用

		} else { // 抬起

			if (up != NULL) up(); // 调用

		}
	}
}