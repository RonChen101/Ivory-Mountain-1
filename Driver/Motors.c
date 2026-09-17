#include "Motors.h"

// 初始化
void Motors_init() {
	EAXSFR();		/* 扩展寄存器访问使能 */
	
	// P14 15 16 17
	P1_MODE_IO_PU(GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
	// P20 21 22 23
	P2_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);
	
	// 默认设置为低
	LF_P = LF_N = RF_P = RF_N = LB_P = LB_N = RB_P = RB_N = 0;
}

/*
 -100      0         100     速度       speed
// 0 ----- 50 ------ 100     占空比
// 后退    停止      前进
// 最快              最快
speed=0 ===> 50
	speed/2 + 50
speed=-100 ===> 0
	speed/2 + 50   
speed=100 ===> 100
	speed/2 + 50  
*/

// -100 --------- 0 --------- 100		速度
//后退最大速度	  0			前进最大速度
// 0 ----------  50 --------- 100		PWM占空比
static char speed2duty(char speed) {
    // speed > 0 前进
    // speed < 0 后退
    return speed / 2 + 50;
}

#define PERIOD (MAIN_Fosc / 1000)
static void	PWM_config(MotorSpeed ms) // -100 --------- 0 --------- 100		速度
{
    PWMx_InitDefine		PWMx_InitStructure;
		
	// 配置PWM1  右后轮  P20  P21
	PWMx_InitStructure.PWM_Mode    		= CCMRn_PWM_MODE1;	//模式,		CCMRn_FREEZE,CCMRn_MATCH_VALID,CCMRn_MATCH_INVALID,CCMRn_ROLLOVER,CCMRn_FORCE_INVALID,CCMRn_FORCE_VALID,CCMRn_PWM_MODE1,CCMRn_PWM_MODE2
	PWMx_InitStructure.PWM_Duty    		= (speed2duty(ms.RB_Speed) / 100.0) * PERIOD;	//PWM占空比时间, 0~Period
	PWMx_InitStructure.PWM_EnoSelect	= (ms.RB_Speed != 0) ? (ENO1P | ENO1N) : 0;	//输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
	PWM_Configuration(PWM1, &PWMx_InitStructure);			//初始化PWM
	// 配置PWM2  左后轮  P22  P23
	PWMx_InitStructure.PWM_Mode    		= CCMRn_PWM_MODE1;	//模式,		CCMRn_FREEZE,CCMRn_MATCH_VALID,CCMRn_MATCH_INVALID,CCMRn_ROLLOVER,CCMRn_FORCE_INVALID,CCMRn_FORCE_VALID,CCMRn_PWM_MODE1,CCMRn_PWM_MODE2
	PWMx_InitStructure.PWM_Duty    		= (speed2duty(ms.LB_Speed) / 100.0) * PERIOD;	//PWM占空比时间, 0~Period
	PWMx_InitStructure.PWM_EnoSelect    = (ms.LB_Speed != 0) ? (ENO2P | ENO2N) : 0;	//输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
	PWM_Configuration(PWM2, &PWMx_InitStructure);			//初始化PWM
	// 配置PWM3  右前轮  P14 P15
	// 0 10 20 30 40   逆时针转(反转)  速度 变小
	// 50              停止
	// 60 70 80 90 100 顺时针转(正转)  速度 变大
	// 0 ----- 50 ------ 100
	// 后退    停止      前进
	// 最快              最快
	PWMx_InitStructure.PWM_Mode    		= CCMRn_PWM_MODE1;	//模式,		CCMRn_FREEZE,CCMRn_MATCH_VALID,CCMRn_MATCH_INVALID,CCMRn_ROLLOVER,CCMRn_FORCE_INVALID,CCMRn_FORCE_VALID,CCMRn_PWM_MODE1,CCMRn_PWM_MODE2
	PWMx_InitStructure.PWM_Duty    		= (speed2duty(ms.RF_Speed) / 100.0) * PERIOD;	//PWM占空比时间, 0~Period
	// PWMx_InitStructure.PWM_EnoSelect    = ENO3P | ENO3N;	//输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
	PWMx_InitStructure.PWM_EnoSelect    = (ms.RF_Speed != 0) ? (ENO3P | ENO3N) : 0;	//输出通道选择
	
	PWM_Configuration(PWM3, &PWMx_InitStructure);	
	// 配置PWM4  左前轮  P16  P17
    PWMx_InitStructure.PWM_Mode    		= CCMRn_PWM_MODE1;	//模式,		CCMRn_FREEZE,CCMRn_MATCH_VALID,CCMRn_MATCH_INVALID,CCMRn_ROLLOVER,CCMRn_FORCE_INVALID,CCMRn_FORCE_VALID,CCMRn_PWM_MODE1,CCMRn_PWM_MODE2
    PWMx_InitStructure.PWM_Duty    		= (speed2duty(ms.LF_Speed) / 100.0) * PERIOD;	//PWM占空比时间, 0~Period
    PWMx_InitStructure.PWM_EnoSelect    = (ms.LF_Speed != 0) ? (ENO4P | ENO4N) : 0;	//输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
    PWM_Configuration(PWM4, &PWMx_InitStructure);

	// 配置PWMA
    PWMx_InitStructure.PWM_Period   = PERIOD - 1;			//周期时间,   0~65535
    PWMx_InitStructure.PWM_DeadTime = 0;					//死区发生器设置, 0~255
    PWMx_InitStructure.PWM_MainOutEnable= ENABLE;			//主输出使能, ENABLE,DISABLE
    PWMx_InitStructure.PWM_CEN_Enable   = ENABLE;			//使能计数器, ENABLE,DISABLE
    PWM_Configuration(PWMA, &PWMx_InitStructure);			//初始化PWM通用寄存器,  PWMA,PWMB

	// 切换PWM通道
	PWM1_SW(PWM1_SW_P20_P21);			//PWM1_SW_P10_P11,PWM1_SW_P20_P21,PWM1_SW_P60_P61
	PWM2_SW(PWM2_SW_P22_P23);			//PWM2_SW_P12_P13,PWM2_SW_P22_P23,PWM2_SW_P62_P63
	PWM3_SW(PWM3_SW_P14_P15);			//PWM3_SW_P14_P15,PWM3_SW_P24_P25,PWM3_SW_P64_P65
	PWM4_SW(PWM4_SW_P16_P17);			//PWM4_SW_P16_P17,PWM4_SW_P26_P27,PWM4_SW_P66_P67,PWM4_SW_P34_P33

	// 初始化PWMA的中断
    NVIC_PWM_Init(PWMA,DISABLE,Priority_0);
}
	

// speed：速度 0~100  mode： LEFT_M左前 , MID_M前进 , RIGHT_M右前
void Motors_forward(char speed, MotorsMode mode) {
	MotorSpeed ms = {0}; // 所有成员初始化为0
	
	if (mode == LEFT_M){ // 左
		ms.LF_Speed = 0;
		ms.LB_Speed = speed;
		ms.RF_Speed = speed;
		ms.RB_Speed = 0;
	} else if (mode == RIGHT_M) { // 右
		ms.LF_Speed = speed;
		ms.LB_Speed = 0;
		ms.RF_Speed = 0;
		ms.RB_Speed = speed;
		
	} else { // 中间
		ms.LF_Speed = speed;
		ms.LB_Speed = speed;
		ms.RF_Speed = speed;
		ms.RB_Speed = speed;
	}
	// 调用pwm, 把参数传递进去
	PWM_config(ms);
}

// speed：速度 0~100  mode： LEFT_M左后 , MID_M后退 , RIGHT_M右后
void Motors_backward(char speed , MotorsMode mode) {
	MotorSpeed ms = {0}; // 所有成员初始化为0
	
	if (mode == LEFT_M){ // 左
		ms.LF_Speed = -speed;
		ms.LB_Speed = 0;
		ms.RF_Speed = 0;
		ms.RB_Speed = -speed;
	} else if (mode == RIGHT_M) { // 右
		ms.LF_Speed = 0;
		ms.LB_Speed = -speed;
		ms.RF_Speed = -speed;
		ms.RB_Speed = 0;
		
	} else { // 中间
		ms.LF_Speed = -speed;
		ms.LB_Speed = -speed;
		ms.RF_Speed = -speed;
		ms.RB_Speed = -speed;
	}
	// 调用pwm, 把参数传递进去
	PWM_config(ms);
}

// speed：速度 0~100  mode： LEFT_M左平移 ，RIGHT_M右平移
void Motors_translate(char speed , MotorsMode mode) {
	MotorSpeed ms = {0}; // 给结构体所有成员赋值为0
	
	if (mode == LEFT_M) { // 左
		ms.LF_Speed = -speed;
		ms.LB_Speed = speed;
		ms.RF_Speed = speed;
		ms.RB_Speed = -speed;
	} else if (mode == RIGHT_M) { // 右
		ms.LF_Speed = speed;
		ms.LB_Speed = -speed;
		ms.RF_Speed = -speed;
		ms.RB_Speed = speed;
	}
	
	PWM_config(ms);
}

// speed：速度 0~100  mode： LEFT_M向左旋转(逆时针) , RIGHT_M向右旋转(顺时针)
void Motors_around(char speed , MotorsMode mode) {
	MotorSpeed ms = {0}; // 给结构体所有成员赋值为0
	
	if (mode == LEFT_M) { // 左(逆时针)
		ms.LF_Speed = -speed;
		ms.LB_Speed = -speed;
		ms.RF_Speed = speed;
		ms.RB_Speed = speed;
	} else if (mode == RIGHT_M) { // 右(顺时针)
		ms.LF_Speed = speed;
		ms.LB_Speed = speed;
		ms.RF_Speed = -speed;
		ms.RB_Speed = -speed;
	}
	
	PWM_config(ms);
}

// speed：速度 0~100  mode： LEFT_M左转 , RIGHT_M右转
void Motors_turn(char speed ,  MotorsMode mode) {
	MotorSpeed ms = {0}; // 给结构体所有成员赋值为0
	
	if (mode == LEFT_M) { // 左
		ms.LF_Speed = 0;
		ms.LB_Speed = 0;
		ms.RF_Speed = speed;
		ms.RB_Speed = speed;
	} else if (mode == RIGHT_M) { // 右
		ms.LF_Speed = speed;
		ms.LB_Speed = speed;
		ms.RF_Speed = 0;
		ms.RB_Speed = 0;
	}
	
	PWM_config(ms);
}

// 停止
void Motors_stop(){
	MotorSpeed ms = {0}; // 所有成员初始化为0
	// 调用pwm, 把参数传递进去
	PWM_config(ms);
}

// value不能是char类型，数据会溢出
// 限制速度值，只能在 ~100 - 100 区间
static char LimitSpeed(int value){ 
	if (value > 100) return 100;
	else if (value < -100) return -100;
	
	return value;
}

#define LIMIT  0.3
// x, y
void Motors_move(char x, char y) {
	MotorSpeed ms = {0}; // 给结构体所有成员赋值为0
	
	ms.LF_Speed = LIMIT * LimitSpeed(x+y);
	ms.LB_Speed = LIMIT * LimitSpeed(y-x);
	ms.RF_Speed = LIMIT * LimitSpeed(y-x);
	ms.RB_Speed = LIMIT * LimitSpeed(x+y);


	PWM_config(ms);
}
