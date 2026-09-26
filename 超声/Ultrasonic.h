#ifndef __ULTRISONIC_H__
#define __ULTRISONIC_H__

#include 	"GPIO.h"
#include	"Delay.h"

// trig：单片机发送信号，高速超声波的模组去测距
// echo：高速超声波的模组响应信号
#define 	TRIG	P47
#define 	ECHO	P33

void Ultrasonic_init();

// 返回值为char，因为有负数，代表不同的状态，返回0，才代表成功获取距离
char Ultrasonic_get_distance(float *distance);

#endif