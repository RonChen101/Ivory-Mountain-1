#include "WS2812.h"

// ============================================================
// 可配置参数
// ============================================================
#define LED_NUM   60            // 灯珠数量：60 颗

// 每个 WS2812 数据位用 4 个 SPI 位表示，60 颗灯需要 60*24*4/8 = 720 字节
// 加上复位低电平的字节，缓冲区稍大一些
static unsigned char xdata spi_buffer[800];

// ============================================================
// SPI 初始化（P1.3 MOSI，P1.4 MISO 输出 0，P1.5 SCLK）
// 关键：MISO 设为推挽输出低电平，MOSI 空闲时就会输出低电平
// ============================================================
static void SPI_Init(void)
{
    P_SW2 |= 0x80;              // 使能扩展寄存器访问

    // ---------- 引脚配置 ----------
    P1M1 &= ~0x38;              // P1.3 (MOSI), P1.4 (MISO), P1.5 (SCLK) 推挽输出
    P1M0 |=  0x38;

    // MISO 输出低电平，MOSI 发送完成后会跟随 MISO 状态
    P14 = 0;

    // ---------- SPI 配置 ----------
    // SPCTL: SSIG=1, SPEN=1, DORD=0, MSTR=1, CPOL=0, CPHA=0, SPR=11 (4分频)
    // 24MHz / 4 = 6MHz SPI 时钟，每个 SPI 位约 167ns
    SPCTL = 0xD3;               // 1101 0011
    SPSTAT = 0xC0;              // 清除标志位
}

// ============================================================
// 发送一个字节（查询方式）
// ============================================================
static void SPI_SendByte(unsigned char dat)
{
    SPDAT = dat;                // 写入数据，SPI 自动发送
    while (!(SPSTAT & 0x80));   // 等待发送完成
    SPSTAT = 0x80;              // 清除 SPIF 标志
}

// ============================================================
// 把一个 WS2812 数据位编码成半个 SPI 字节
// 逻辑 1 → 0xC0（高4位为 1100，高电平时间长）
// 逻辑 0 → 0xFC（高4位为 1111，低电平时间长）
// ============================================================
static void EncodeBit(unsigned char bit_val, unsigned char *buf, unsigned int *idx)
{
    if (bit_val)
        buf[*idx] = 0xC0;       // 逻辑 1
    else
        buf[*idx] = 0xFC;       // 逻辑 0
    (*idx)++;
}

// ============================================================
// 把一颗灯的颜色编码到 SPI 缓冲区
// WS2812 顺序：G → R → B
// ============================================================
static void EncodeColor(unsigned char r, unsigned char g, unsigned char b,
                        unsigned char *buf, unsigned int *idx)
{
    unsigned char i;
    unsigned char dat;

    // 绿色
    dat = g;
    for (i = 0; i < 8; i++) {
        EncodeBit(dat & 0x80, buf, idx);
        dat <<= 1;
    }

    // 红色
    dat = r;
    for (i = 0; i < 8; i++) {
        EncodeBit(dat & 0x80, buf, idx);
        dat <<= 1;
    }

    // 蓝色
    dat = b;
    for (i = 0; i < 8; i++) {
        EncodeBit(dat & 0x80, buf, idx);
        dat <<= 1;
    }
}

// ============================================================
// 对外唯一接口：初始化 + 设置全部灯为同一颜色
// ============================================================
void WS2812_Init_And_Set(unsigned char r, unsigned char g, unsigned char b)
{
    unsigned int idx = 0;
    unsigned int i;

    SPI_Init();                 // 初始化 SPI

    // ---------- 编码 60 颗灯的颜色数据 ----------
    for (i = 0; i < LED_NUM; i++) {
        EncodeColor(r, g, b, spi_buffer, &idx);
    }

    EA = 0;                     // 关中断，保护时序

    // ---------- 发送数据 ----------
    for (i = 0; i < idx; i++) {
        SPI_SendByte(spi_buffer[i]);
    }

    // ---------- 复位信号：拉低 MOSI 保持 >50μs ----------
    // 通过发送 0x00 让 MOSI 保持低电平（MISO 已设为 0）
    for (i = 0; i < 100; i++) {
        SPI_SendByte(0x00);
    }

    EA = 1;                     // 恢复中断
}