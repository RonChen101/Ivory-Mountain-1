#ifndef __NIXIE_H__
#define __NIXIE_H__

#include "GPIO.h"

// 声明宏变量
#define NIX_DI		P44	// 数据输入
#define NIX_SCK		P42	// 移位寄存器
#define NIX_RCK		P43	// 锁存寄存器

// 初始化
void Nixie_init();

// num: 控制显示的什么内容
// idx: 控制显示哪几个显示
void Nixie_show(u8 num, u8 idx);

// num 对应的内容在数组的位置(索引)，配合自定义码表
// 码表https://www.yuque.com/mikejiangsky/ca1cyy/sk0cxccrvps7gnuv#ZxeBQ
// 除了0~9，刚好和下标一致，其它的内容，需要查表
// idx  显示在屏幕上的位置(1 -> 8)，只有1个数码管显示
void Nixie_display(u8 num, u8 idx);

void Nixie_clear(); // 灯灭

#endif