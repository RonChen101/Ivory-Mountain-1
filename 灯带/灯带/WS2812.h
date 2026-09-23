#ifndef __WS2812_H
#define __WS2812_H

#include "Config.h"

// ============================================================
// WS2812 驱动（SPI 查询发送方案）
// 适用：STC8H8K64U，引脚：P1.3 (MOSI)
// ============================================================

void WS2812_Init_And_Set(unsigned char r, unsigned char g, unsigned char b);

#endif