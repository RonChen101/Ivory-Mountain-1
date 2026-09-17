#ifndef ___MOTORS_H__
#define ___MOTORS_H__

#include 	"GPIO.h"
#include 	"STC8H_PWM.h"
#include	"NVIC.h"
#include 	"Switch.h"

// 左前轮 left  forward
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

typedef struct{
	char LF_Speed;	// 左前轮速度
	char LB_Speed;	// 左后轮速度
	char RF_Speed;	// 右前轮速度
	char RB_Speed;	// 右后轮速度
}MotorSpeed;

typedef enum{
	LEFT_M, MID_M , RIGHT_M
}MotorsMode;

// 初始化
void Motors_init();

// speed：速度 0~100  mode： LEFT_M左前 , MID_M前进 , RIGHT_M右前
void Motors_forward(char speed, MotorsMode mode);

// speed：速度 0~100  mode： LEFT_M左后 , MID_M后退 , RIGHT_M右后
void Motors_backward(char speed , MotorsMode mode);

// speed：速度 0~100  mode： LEFT_M左平移 ，RIGHT_M右平移
void Motors_translate(char speed , MotorsMode mode);

// 顺时针 (Clockwise): 想象一个时钟，指针从12点走向1点、2点、3点。在时钟的上半部分，指针是向右移动的。所以“向右转”就是顺时针。
// 逆时针 (Counter-clockwise): 与时钟指针相反的方向，从12点走向11点、10点。在时钟的上半部分，指针是向左移动的。所以“向左转”就是逆时针。
// speed：速度 0~100  mode： LEFT_M向左旋转(逆时针) , RIGHT_M向右旋转(顺时针)
void Motors_around(char speed , MotorsMode mode);

// speed：速度 0~100  mode： LEFT_M左转 , RIGHT_M右转
void Motors_turn(char speed ,  MotorsMode mode);

void Motors_move(char x, char y); // 声明

// 停止
void Motors_stop();

#endif