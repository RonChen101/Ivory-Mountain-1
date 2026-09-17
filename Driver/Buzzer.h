#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "Config.h"

void Buzzer_init(void);

void Buzzer_play(u16 hz_value);

// 按照指定音调播放声音
void Buzzer_beep(u16 tone);

void Buzzer_stop(void);

void Buzzer_demo_2tiger(); // 2只老虎

void Buzzer_alarm();  // 警告声

#endif