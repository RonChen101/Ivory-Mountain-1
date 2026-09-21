#ifndef ___MOTORS_H__
#define ___MOTORS_H__

#include 	"GPIO.h"
#include 	"STC8H_PWM.h"
#include	"NVIC.h"
#include 	"Switch.h"

// 左前轮 left forward
#define 	LF_P		P16
#define 	LF_N		P17

// 右前轮 right forward
#define 	RF_P		P14
#define 	RF_N		P15

// 左后轮 left backward
#define 	LB_P		P22
#define 	LB_N		P23

// 右后轮 right backward
#define 	RB_P		P20
#define 	RB_N		P21

// 四轮速度，取值 -100~100（负值为后退，0 停止，正值为前进）
typedef struct{
	char LF_Speed;	// 左前轮速度
	char LB_Speed;	// 左后轮速度
	char RF_Speed;	// 右前轮速度
	char RB_Speed;	// 右后轮速度
}MotorSpeed;

// 动作作用范围：LEFT_M 左侧轮组 , MID_M 全部四轮 , RIGHT_M 右侧轮组
typedef enum{
	LEFT_M, MID_M , RIGHT_M
}MotorsMode;

// 初始化：配置引脚为准双向口，所有轮子停止
void Motors_init();

// speed：速度 0~100  mode：LEFT_M左半侧 , MID_M全部 , RIGHT_M右半侧
void Motors_forward(char speed, MotorsMode mode);

// speed：速度 0~100  mode：LEFT_M左半侧 , MID_M全部 , RIGHT_M右半侧
void Motors_backward(char speed , MotorsMode mode);

// 平移：speed 速度 0~100  mode：LEFT_M向左平移 , RIGHT_M向右平移（其它模式不动作）
void Motors_translate(char speed , MotorsMode mode);

// 原地旋转：
// 顺时针 (Clockwise)：指针沿钟面从 12 点转向 1 点、2 点、3 点的方向
// 逆时针 (Counter-clockwise)：与顺时针相反，从 12 点转向 11 点、10 点的方向
// speed：速度 0~100  mode：LEFT_M 逆时针旋转 , RIGHT_M 顺时针旋转
void Motors_around(char speed , MotorsMode mode);

// 单侧差速转向：speed 速度 0~100  mode：LEFT_M左转（仅右轮前进） , RIGHT_M右转（仅左轮前进）
void Motors_turn(char speed ,  MotorsMode mode);

// 全向移动：x 为横向分量，y 为纵向分量，内部自动限幅并分配四轮速度
void Motors_move(char x, char y);

// 唯一PWM入口：直接设置四轮速度 -100~100（LF左前 LB左后 RF右前 RB右后）
// 所有动作函数最终都经由它输出，跨任务调用只允许通过这一个写者
void Motors_apply(char lf_speed, char lb_speed, char rf_speed, char rb_speed);

// 停止：四轮速度清零
void Motors_stop();

#endif
