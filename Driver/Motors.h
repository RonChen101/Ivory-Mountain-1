#ifndef ___MOTORS_H__
#define ___MOTORS_H__

#include 	"GPIO.h"
#include 	"STC8H_PWM.h"
#include	"NVIC.h"
#include 	"Switch.h"

// ��ǰ�� left  forward
#define 	LF_P		P16
#define 	LF_N		P17

// ��ǰ�� right forward
#define 	RF_P		P14
#define 	RF_N		P15

// ����� left backward
#define 	LB_P		P22
#define 	LB_N		P23

// �Һ��� right backward
#define 	RB_P		P20
#define 	RB_N		P21

typedef struct{
	char LF_Speed;	// ��ǰ���ٶ�
	char LB_Speed;	// ������ٶ�
	char RF_Speed;	// ��ǰ���ٶ�
	char RB_Speed;	// �Һ����ٶ�
}MotorSpeed;

typedef enum{
	LEFT_M, MID_M , RIGHT_M
}MotorsMode;

// ��ʼ��
void Motors_init();

// speed���ٶ� 0~100  mode�� LEFT_M��ǰ , MID_Mǰ�� , RIGHT_M��ǰ
// 唯一PWM入口：直接设置四轮速度 -100~100（LF左前 LB左后 RF右前 RB右后）
// 所有动作函数最终都经由它输出，跨任务调用只允许通过这一个写者
void Motors_apply(char lf_speed, char lb_speed, char rf_speed, char rb_speed);

void Motors_forward(char speed, MotorsMode mode);

// speed���ٶ� 0~100  mode�� LEFT_M��� , MID_M���� , RIGHT_M�Һ�
void Motors_backward(char speed , MotorsMode mode);

// speed���ٶ� 0~100  mode�� LEFT_M��ƽ�� ��RIGHT_M��ƽ��
void Motors_translate(char speed , MotorsMode mode);

// ˳ʱ�� (Clockwise): ����һ��ʱ�ӣ�ָ���12������1�㡢2�㡢3�㡣��ʱ�ӵ��ϰ벿�֣�ָ���������ƶ��ġ����ԡ�����ת������˳ʱ�롣
// ��ʱ�� (Counter-clockwise): ��ʱ��ָ���෴�ķ��򣬴�12������11�㡢10�㡣��ʱ�ӵ��ϰ벿�֣�ָ���������ƶ��ġ����ԡ�����ת��������ʱ�롣
// speed���ٶ� 0~100  mode�� LEFT_M������ת(��ʱ��) , RIGHT_M������ת(˳ʱ��)
void Motors_around(char speed , MotorsMode mode);

// speed���ٶ� 0~100  mode�� LEFT_M��ת , RIGHT_M��ת
void Motors_turn(char speed ,  MotorsMode mode);

void Motors_move(char x, char y); // ����

// ֹͣ
void Motors_stop();

#endif