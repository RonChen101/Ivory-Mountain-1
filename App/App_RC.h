#ifndef __APP_RC_H__
#define __APP_RC_H__

#include "App.h"

// 串口1接收任务：把串口1收到的数据转发到串口2
void uart1_recv_task(void);

// 串口2接收任务：串口2就是蓝牙，解析帧后交给车辆状态机
void uart2_recv_task(void);

#endif
