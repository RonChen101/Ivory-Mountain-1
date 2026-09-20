# Ivory-Mountain-1 蓝牙巡线小车

基于 **STC8H8K64U**（8051 内核，24MHz）的四轮小车，使用 **RTX-51 Tiny** 实时系统多任务调度，支持蓝牙遥控（摇杆全向移动）与 5 路红外巡线两种驾驶模式，带蜂鸣器、车灯、超声波测距、电池电压检测等外设。

## 功能一览

- **蓝牙遥控**：摇杆全向移动（前进/后退/平移）、B/C 键原地旋转、蜂鸣器、车灯开关
- **巡线模式**：5 路红外巡迹，差速转向自动循黑线，蓝牙 D 键或车上按键一键开关
- **互斥保护**：巡线开启时自动屏蔽手动驾驶
- **调试外设**：电池电压 ADC 采样、HC-SR04 超声波测距、串口调试打印

## 目录结构

```
User/
  main.c              RTX 主任务（task 0）：初始化 + 创建任务
App/                 业务层
  App_System.c        系统初始化编排（IO / 串口 / 电机 / 巡迹）
  App_Vehicle.c       ★ 车辆状态机：唯一的电机"指挥官"
  App_Track.c         巡线控制律（位图→偏移→差速）
  App_RC.c            蓝牙协议解析（帧校验 → 意图）
  App_Key.c           车上按键 → 只发意图
Driver/              硬件抽象层
  Motors.c            四路 PWM 电机驱动，Motors_apply() 唯一 PWM 入口
  TrackSensor.c       5 路巡迹传感器（引脚下沉于此）
  Key.c / Light.c / Buzzer.c / Battery.c / Ultrasonic.c
Lib/                 STC 官方外设库（UART/PWM/GPIO/ADC/定时器等，勿改动）
```

**分层规则**：调用关系只能自上而下 `App → Driver → Lib`；
巡线任务的创建/删除只发生在 `App_Vehicle.c`；
对电机的写入只经 `Motors_apply()` 一个出口。

## 架构与数据流

```
蓝牙ISR → RX2_Buffer → App_RC（帧校验/边缘检测）─┐
按键扫描 → App_Key ──────────────────────────────┤→ App_Vehicle 状态机 → Motors_apply → PWM
                                                 │        ↑
                        Driver/TrackSensor ──────┘ (仅巡线模式)
```

### 车辆状态机（App_Vehicle）

| 模式 | 含义 | 行为 |
|------|------|------|
| `VEH_IDLE` | 上电初始 | 等待指令 |
| `VEH_MANUAL` | 手动驾驶 | 响应摇杆 / 旋转键 |
| `VEH_TRACKING` | 巡线中 | 屏蔽一切手动指令 |

切换全部通过 `Vehicle_set_mode()`（幂等，重复设置不会重复创建/删除任务）。

### RTX-51 Tiny 任务表

| 任务 ID | 函数 | 文件 | 周期 | 说明 |
|---------|------|------|------|------|
| 0 | `main_task` | User/main.c | — | 初始化后自删除 |
| 1 | `track_task` | App/App_Track.c | 15ms | 巡线，由状态机按需创建/删除 |
| 2 | `uart1_recv_task` | App/App_RC.c | 5ms | 串口1 收→串口2 转发 |
| 3 | `uart2_recv_task` | App/App_RC.c | 5ms | 蓝牙帧接收与解析 |
| 4 | `key_task` | App/App_Key.c | 10ms | 按键扫描（默认未创建） |

## 引脚分配

| 外设 | 引脚 | 说明 |
|------|------|------|
| 电机 PWM（左前） | P16 / P17 | PWM4 |
| 电机 PWM（右前） | P14 / P15 | PWM3 |
| 电机 PWM（左后） | P22 / P23 | PWM2 |
| 电机 PWM（右后） | P20 / P21 | PWM1 |
| 巡迹传感器 ×5 | P00 ~ P04 | 高电平 = 压黑线 |
| 车上按键 | P05 | 低电平按下 |
| 车灯 | P07（左）/ P52（右） | |
| 电池电压检测 | P13 | ADC 通道 3 |
| UART1（调试） | P30 / P31 | 115200 |
| UART2（蓝牙） | P10 / P11 | 115200 |
| 蜂鸣器 | P34 | PWM8（PWMB 组） |
| 超声波 TRIG / ECHO | P47 / P33 | |

## 蓝牙协议（8 字节帧，115200）

```
索引   0    1    2    3    4    5    6    7
内容  0xDD 0x77  x    y    A    B    C    D
```

- **x / y**：摇杆偏移量，8bit 有符号（-128~127），直接映射差速
- **A**：蜂鸣器 + 车灯开关（边缘触发，按一次切换）
- **B / C**：左旋 / 右旋（电平触发，按住生效）
- **D**：巡线开/关（边缘触发）

数据示例：`DD 77 EF 50 01 00 01 00`（x=-17，y=80，A 按下）

接收端以"超时 + 帧长 ≥ 8 + 帧头校验"三重判定完整帧；半截帧直接丢弃。

## 构建与烧录

1. **工具链**：Keil C51 + [EIDE (VSCode 插件)](https://em-ide.com/)，工程配置在 `.eide/eide.yml`
2. **关键配置**：RAM/ROM 模式 LARGE；链接器 RTX = `RTX-Tiny`；主频 24MHz（`Lib/Config.h` 的 `MAIN_Fosc`）
3. EIDE 中点击 Build（产物在 `build/`），烧录用 STC-ISP / 串口下载

> 注意：根目录的 `stc8h8k64u.uvproj` 为旧版 Keil 工程，未包含 App 层文件；请以 EIDE 工程为准。

## 调试技巧

- 串口1（P30/P31，115200）输出调试信息（`printf` 已重定向）
- 巡线偏移量打印：取消 `App_Track.c` 中 `printf("pos = %d\n", pos);` 的注释
- 速度调节：巡线速度在 `App_Track.c` 的 `track_task`（经验值：低于 18 压差不够，车不动）
- 电机混控比例：`Driver/Motors.c` 的 `LIMIT`（当前 0.3，全速压低系数）

## 已知事项 / 待办

- [ ] 摇杆响应优化：缩短 `TimeOutSet2`（[Lib/UART.h](Lib/UART.h)）、`Motors_apply` 改整数直写 CCR、加死区与非线性曲线
- [ ] 差速混合归一化（斜向 45° 时输出被截顶，速度损失约 30%）
- [ ] 新增文件为 UTF-8 编码，旧文件为 GBK；Keil 直接打开新文件注释可能显示乱码（VSCode 正常）
