#include "Ultrasonic.h"

void Ultrasonic_init() {
	// TRIG	P47  准双向 或 推挽输出
	//推挽输出	
	P4_MODE_OUT_PP(GPIO_Pin_7);
	// TRIG 拉低
	TRIG = 0;
	
	// ECHO	P33  输入   高阻输入
	P3_MODE_IN_HIZ(GPIO_Pin_3);
}

void Delay10us(void)	//@24.000MHz
{
	unsigned char data i;

	i = 78;
	while (--i);
}



// 返回值为char，因为有负数，代表不同的状态，返回0，才代表成功获取距离
char Ultrasonic_get_distance(float *distance) {
	u16 cnt = 0;
	
	// 1. 至少给trig 10us(给个2个10us)高电平时间，再拉低
	TRIG = 1;
	Delay10us(); Delay10us();  // 20us
	TRIG = 0;
	
	// 2. 计算echo低电平的时间，(当echo变高电平时，退出循环)
	// 假如超过 500 * 10 = 5000us 都没有变为高电平(防止没有插上超声波模块，导致程序卡死)，也要退出循环
	while(ECHO == 0 && cnt < 500) {
		cnt++;
		Delay10us();
	}
	if (cnt >= 500) return -1;
	
	// 3. 计算echo高电平时间
	// 如果 cnt 达到了 3000，如果高电平超过了 30ms，说明目标太远了（大于 5 米），没有回波
	cnt = 0;
	while(ECHO == 1 && cnt < 3000) {
		cnt++;
		Delay10us();
	}
	if (cnt >= 3000) return -2;
	// printf("cnt = %d\n", (int)cnt);
	
	// 4. 计算距离：测试距离= (高电平时间*声速(340M/S))/2 要除以2，因为声音有来回
	// cnt 是高电平的时间  1个cnt 为 10 us 为0.01ms
	// dis = ((cnt * 0.01)ms * 340m/s) / 2
	// dis = ((cnt * 0.01)ms * 34000cm/1000ms) / 2
	// dis = ((cnt * 0.01)ms * 34cm/ms) / 2
	*distance = ((cnt * 0.01) * 34) / 2;
	// 距离范围  2cm ~ 400cm  不在这个范围，无法测量，下面代码可能没有作用
	if (*distance < 2 || *distance > 400) return -3;
	
	
	return 0;
}