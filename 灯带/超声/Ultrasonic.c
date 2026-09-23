#include "Ultrasonic.h"


void Ultrasonic_init(){
    // TRIG P47设置 推挽输出 或 准双向
    P4_MODE_OUT_PP(GPIO_Pin_7);
    //拉低 P47
    TRIG = 0;
    // ECHO P33 高阻输入 : 输入
    P3_MODE_IN_HIZ(GPIO_Pin_3);
}
//isp串口工具生成10us的delay
void Delay10us(void)	//@24.000MHz
{
	unsigned char data i;

	i = 78;
	while (--i);
}


// 返回值为char，因为有负数，代表不同的状态，返回0，才代表成功获取距离
char Ultrasonic_get_distance(float *distance){
    //定义超时计数变量
    u16 cnt = 0;
    //给TRIG一个最少10us的高电平时间
    TRIG = 1;
    Delay10us();Delay10us(); //20us
    //拉低
    TRIG = 0;
    //计算echo低电平时间 变高 或 超时5000us 时退出计时
    while(ECHO == 0 && cnt <500){
        cnt++;
        Delay10us();
    }
    if (cnt >= 500) return -1; //返回退出原因 超时
    //计算echo高电平时间若计时超过30ms则为无效数据退出
    cnt = 0;
    while(ECHO == 1 && cnt <3000){
        cnt++;
        Delay10us();
    }
    if (cnt >= 3000) return -2;  //返回退出原因 超距 无回波
    //计算距离: 测距 = (高电平时间*升速(340m/s))/ 2 除2声音有来回
    //cnt 高电平时间 1个cnt为 10us 为0.01ms
    //dis = ((cnt * 0.01) * 34000cm/1000ms) / 2
    //dis = ((cnt * 0.01) * 34cm/ms) / 2
    *distance = ((cnt * 0.01) * 34) / 2;
    // 距离范围为2~ 400cm 超距无法测量
    if (*distance < 2 || *distance >400) return -3;//超距无法测量
        
    return 0;
}