#include "App.h"
#include "TrackSensor.h"
#include "Motors.h"


/*
App_Track
巡线控制：读传感器位图(滤波/坏灯屏蔽) -> 加权平均求偏移 -> 比例差速
- 环境光滤波：每周期每路多次采样做多数表决，滤除环境光瞬时毛刺
- 坏灯容错：某路值长时间不变判为坏灯并屏蔽，坏1~2路仍可循迹
- 比例差速：偏差越大内外轮速差越大，转向连续渐变，消除蛇形
- 丢线容错：按最近偏差方向原地找线，超时停车保护
*/

// ---- 可调参数（上车调参只改这里）----
#define TRACK_BASE_SPEED    25      // 巡线基础速度
#define TRACK_KP_NUM        1       // 比例系数 Kp = NUM/DEN，diff = Kp * pos
#define TRACK_KP_DEN        3
#define TRACK_SAMPLE_CNT    5       // 每周期每路采样次数（环境光滤波）
#define TRACK_VOTE_NEED     3       // 采样中 >=3 次为1才判压线（多数表决）
#define TRACK_LOST_DEBOUNCE 3       // 连续3周期全灭才确认丢线（约45ms防抖）
#define TRACK_LOST_TIMEOUT  30      // 确认丢线后再经30周期（共约500ms）仍无线则停车
#define TRACK_STUCK_LIMIT   250     // 传感器值连续不变周期数阈值（约3.8s）判坏灯
#define TRACK_SEARCH_SPEED  15      // 找线旋转速度（约基础速度的60%）

// 5路传感器位置权重（bit0~bit4 = 左1~右2，左偏为负）
static int code pos_w[5] = {-64, -32, 0, 32, 64};

// 坏灯检测状态
static u8 same_cnt[5];          // 每路值不变的连续周期数
static u8 sens_dead;            // 坏灯屏蔽位图（置位 = 该路已屏蔽）

// 初始化
void Track_init(void) {
	TrackSensor_init();
}

// 每周期对5路快速采样多次做多数表决，滤除环境光造成的瞬时毛刺
static u8 Track_read_filtered(void) {
	u8 cnt[5];
	u8 i, j, b, filtered = 0;

	for (j = 0; j < 5; j++) cnt[j] = 0;
	for (i = 0; i < TRACK_SAMPLE_CNT; i++) {
		b = TrackSensor_read();
		for (j = 0; j < 5; j++) {
			if (b & (1 << j)) cnt[j]++;
		}
	}
	for (j = 0; j < 5; j++) {
		if (cnt[j] >= TRACK_VOTE_NEED) filtered |= (u8)(1 << j);
	}
	return filtered;
}

// 坏灯检测：某路值连续 TRACK_STUCK_LIMIT 周期不变则屏蔽，值一变化立即恢复
static void Track_update_dead(u8 cur, u8 prev) {
	u8 j, msk;

	for (j = 0; j < 5; j++) {
		msk = (u8)(1 << j);
		if ((cur & msk) != (prev & msk)) {
			same_cnt[j] = 0;
			sens_dead &= (u8)~msk;      // 值变化，恢复参与定位
		} else if (same_cnt[j] < TRACK_STUCK_LIMIT) {
			same_cnt[j]++;
			if (same_cnt[j] >= TRACK_STUCK_LIMIT) sens_dead |= msk;
		}
	}
}

// 采样滤波 + 更新坏灯屏蔽，返回有效位图（屏蔽前位图经 *raw_out 带回）
static u8 Track_get_effective(u8 *raw_out) {
	static u8 prev = 0;
	u8 raw = Track_read_filtered();

	Track_update_dead(raw, prev);
	prev = raw;
	*raw_out = raw;
	return raw & (u8)~sens_dead;
}

// 按压线路数加权平均求偏移，仅当 eff 非零时调用
static int Track_calc_pos(u8 eff) {
	u8 j, cnt = 0;
	int sum = 0;

	for (j = 0; j < 5; j++) {
		if (eff & (u8)(1 << j)) {
			sum += pos_w[j];
			cnt++;
		}
	}
	return sum / cnt;
}

// 获取寻迹坐标：左偏为负(-64)，右偏为正(+64)，居中为0
// 无有效压线时返回上一次坐标（保留对外接口，坏灯已自动屏蔽）
int Track_get_position(void) {
	static int last_pos = 0;
	u8 raw, eff;

	eff = Track_get_effective(&raw);
	if (eff != 0) last_pos = Track_calc_pos(eff);
	return last_pos;
}


void track_task() _task_  TRACK_TASK_ID { // 巡线
	int last_err = 0;      // 最近一次有效偏差（<0 线在左）
	u8 blank_cnt = 0;      // 连续全灭周期计数
	int pos, diff, l, r;
	u8 raw, eff;

	while (1) {
		eff = Track_get_effective(&raw);

		if (eff != 0) {
			// ---- 在线上：比例差速，偏差越大内外轮速差越大 ----
			blank_cnt = 0;
			pos = Track_calc_pos(eff);
			last_err = pos;

			diff = pos * TRACK_KP_NUM / TRACK_KP_DEN;
			l = TRACK_BASE_SPEED + diff;    // pos<0 线在左 → 左轮减速右轮加速 → 左转
			r = TRACK_BASE_SPEED - diff;
			// 限幅保护（调大 Kp 或速度后不越界）
			if (l > 100) l = 100; else if (l < -100) l = -100;
			if (r > 100) r = 100; else if (r < -100) r = -100;
			Motors_apply((char)l, (char)l, (char)r, (char)r);
		} else if (raw == 0) {
			// ---- 有效全灭：防抖后按最近偏差方向原地找线，超时停车保护 ----
			blank_cnt++;
			if (blank_cnt >= TRACK_LOST_DEBOUNCE + TRACK_LOST_TIMEOUT) {
				Motors_stop();      // 找线超时，停车保护；重新压线后自动恢复
			} else if (blank_cnt >= TRACK_LOST_DEBOUNCE) {
				// last_err<0 线在左 → 逆时针找线；否则顺时针
				if (last_err < 0) Motors_around(TRACK_SEARCH_SPEED, LEFT_M);
				else              Motors_around(TRACK_SEARCH_SPEED, RIGHT_M);
			}
			// 防抖期内：沿用上一拍输出，不动作
		} else {
			// ---- 只有坏灯报压线：可信度不足 ----
			// 未在找线中 → 保持直行（持续状态，不参与超时）
			// 正在找线中 → 忽略坏灯信号，继续找线
			if (blank_cnt < TRACK_LOST_DEBOUNCE) {
				Motors_apply((char)TRACK_BASE_SPEED, (char)TRACK_BASE_SPEED,
				             (char)TRACK_BASE_SPEED, (char)TRACK_BASE_SPEED);
			}
		}

		// 真正巡线时候，时间不能太长    15ms差不多了
		os_wait2(K_TMO, 3); // 5 * 3 = 15ms   如果调试，时间可以长一点
	}
}
