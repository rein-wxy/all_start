# all_start

本工程旨在使用C板开发时免去繁琐的外设模块配置，快速构建应用代码。

包含：DBUS，imu，usb，can,uart外设配置

dji电机，海泰电机，以及常用的数据滤波，pid处理。

## cubemx引脚配置

### GPIO

- PA_4 PB_0
- 板载陀螺仪 spi通信寄存器

### 定时器

TIM4

- PWM4_CH3 蜂鸣器
- PD_14

TIM5

- PWM5_CH1 CH2  CH3
- PH_10 PH_11 PH_12
- B  G  R LED灯 

TIM10

- pwm_ch1 板载陀螺仪温控加热
- PF6

### Connectivity 通信

CAN1/CAN2

- 电机控制反馈
- 开启NART自动重传  TXFP 报文优先级判断
- PD_0 PD_1(修改c配置默认)
- PB_5 PB_6

SPI1

- 板载陀螺仪通信
- 预分频256//没搞明白
- PA_7 MOSI PB_3 SCK  PB_4 MISO

![img](https://k10kkja70f2.feishu.cn/space/api/box/stream/download/asynccode/?code=NzdmN2Y2NjhiMWRhNGE5NTE4MTQ4ZWU2NjhmNzZhMjBfR0d3YW1pa29XUVNhUU5nVkNhZTQwSEpoQ010YTI5dUNfVG9rZW46VDAxWmJGNUVkbzNsZkR4RFo4emNuMUlibkVyXzE3MTUyNDY3MTQ6MTcxNTI1MDMxNF9WNA)

USART1

- 与视觉通信//后续可能采用虚拟串口
- 开启RT DMA（normal模式）
- PA_9 PB7

USART3

- 遥控器接收
- 开启R DMA（normal模式）//循环
- PC_10 PC_11

USART6

- 与裁判系统通信
- 开启RT DMA（normal模式）
- PG_9 PG_14

## 文件夹分配

### bsp

硬件驱动接口

### application

任务代码
