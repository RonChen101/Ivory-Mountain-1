#include "Nixie.h"

// 索引对应表格参见：
// https://www.yuque.com/mikejiangsky/ca1cyy/sk0cxccrvps7gnuv#ZxeBQ
// 内容0~9，刚好下标也是0~9
u8 code LED_TABLE[] = 
{
	// 0 	1	 2	-> 9	(索引012...9)
	0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,
	// 0.  1. 2. -> 9.	(索引10,11,12....19)
    0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10,
	// . -						(索引20,21)
	0x7F, 0xBF,
	// AbCdEFHJLPqU		(索引22,23,24....33)
	0x88,0x83,0xC6,0xA1,0x86,0x8E,0x89,0xF1,0xC7,0x8C,0x98,0xC1
};

// 初始化
void Nixie_init() {
	// 推挽  驱动能力强
	P4_MODE_OUT_PP(GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4);
	
	Nixie_clear(); // 初始化完，所有数码管默认是灭的
}

// num: 控制显示的什么内容
// idx: 控制显示哪几个显示
void Nixie_show(u8 num, u8 idx) {
	char i;  // 有-操作，不要用u8
	// 先放内容
	for (i = 7; i >= 0; i--) {
		// 取出第i位，给数据引脚赋值
		NIX_DI = num >> i & 1;
		
		// 移位 0 -> 1
		NIX_SCK = 0;
		NOP2();
		NIX_SCK = 1;
		NOP2();
	}
	
	// 再放位置
	for (i = 7; i >= 0; i--) {
		// 取出第i位，给数据引脚赋值
		NIX_DI = idx >> i & 1;
		
		// 移位 0 -> 1
		NIX_SCK = 0;
		NOP2();
		NIX_SCK = 1;
		NOP2();
	}
	
	// 锁存
	NIX_RCK = 0;
	NOP2();
	NIX_RCK = 1;
	NOP2();
}

// num 对应的内容在数组的位置(索引)，配合自定义码表
// 码表https://www.yuque.com/mikejiangsky/ca1cyy/sk0cxccrvps7gnuv#ZxeBQ
// 除了0~9，刚好和下标一致，其它的内容，需要查表
// idx  显示在屏幕上的位置(1 -> 8)，只有1个数码管显示
void Nixie_display(u8 num, u8 idx) {
	Nixie_show(LED_TABLE[num], 1 << (idx - 1));
}

void Nixie_clear() { // 灯灭
	Nixie_show(0xff, 0x00);
}