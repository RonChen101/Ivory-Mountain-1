#ifndef __KEY_H__
#define __KEY_H__

#include "GPIO.h"

#define KEY		P05
#define USE_KEYDOWN	0 
#define USE_KEYUP	0
// 按下抬起函数的声明，如需使用，需打开开关，需用户在合适位置定义
void Key_on_keydown();
void Key_on_keyup();

// 初始化
void Key_init();
// 扫描按键
void Key_scan();

// 扫描按键，函数指针回调函数版本
// 不要和Key_scan()同时使用
void Key_scan2(void (*down)(), void (*up)());

#endif