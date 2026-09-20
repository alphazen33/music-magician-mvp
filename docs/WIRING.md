# 接线与 BOM

## 优先：AtomS3R-CAM AI Chatbot

本配置操作外部 Grove 接口，并可通过 `arm` 启用板载 BMI270；上电默认不启用动作检测，不初始化相机、声音或网络。刷写后原有聊天机器人应用不会同时运行。

官方 CAM Grove 线序为 **黑 GND / 红 5V / 黄 GPIO2 / 白 GPIO1**。默认固件：白线 GPIO1 连接独立马达驱动的逻辑输入；黄线 GPIO2 连接常开瞬时按钮，另一端接 GND（内部上拉）。不需要按钮时，直接使用 USB 串口 `play hello`。

| 器件 | 数量 | 规格/用途 |
| --- | ---: | --- |
| AtomS3R-CAM（可带 Atomic Voice Base） | 1 | 已有设备；使用 `m5atoms3r_cam` 环境 |
| Grove HY2.0 4P 转杜邦线 | 1 | 按官方颜色核对，杜绝将 5V 接 GPIO |
| 3V 硬币 ERM 振动马达 | 1 | 推荐 10 mm 级；卖家需给额定/堵转电流，非 LRA |
| 逻辑电平 N-MOSFET 驱动板 | 1 | 3.3V 逻辑兼容；含续流二极管/栅极下拉，额定电流高于堵转电流 |
| 稳压 3.0V 马达电源 | 1 | 额定电流高于马达堵转电流；与主控共地 |
| 常开按钮 | 可选 1 | GPIO2–GND；30 ms 固件消抖 |
| 数据 USB-C 线、面包板、连接线 | 各 1 | USB 数据线不能仅充电 |

淘宝可复制搜索：`硬币振动马达 3V 1030 ERM`、`MOS管驱动模块 3.3V 逻辑 续流二极管`、`HY2.0 4P Grove 转杜邦线`、`3V 稳压电源 模块`。优先广东现货；逐项核实卖家发货地、库存、当天截单时间、最早到货日；本仓库未验证实时库存或报价，也未下单。

通用分立接法：

```text
马达额定电源 + ── 马达 +
马达 - ── MOSFET Drain
MOSFET Source ── GND ── 主控 GND
GPIO1 ── 100Ω ── MOSFET Gate
Gate ── 100kΩ ── GND
续流二极管：阴极(带条纹端)接马达 +，阳极接马达 -
电源去耦电容并联在马达电源与 GND 之间
```

不要把马达直接接 GPIO。不要把 Grove 红线 5V 直接接 3V 马达；PWM 限制不等于降压器。不要把 LRA 接到这里的单管 ERM 电路；LRA 需要匹配驱动（例如 DRV2605L），本版本没有该驱动。

可选成品模块：M5 **Unit Vibrator U059** 已内置 N-MOSFET，官方推荐 PWM 10 kHz，DIN 在黄色线。使用预定义环境 `m5atoms3r_cam_u059` 即自动选择 G2 马达、G1 可选外接按钮、10 kHz；无需改源代码。先确认 5V 供电能力；官方列出的 50% PWM 工况为 424.35 mA，不能假定任意 USB 口/供电线路都足够。只用串口触发时不需要分接按钮。

CAM 的底座占用 GPIO5/6/7/8/38/39；摄像头用 GPIO3/4/9/10/11/12/13/14/17/18/21/40/42/46/48。不要复用这些线接本项目外设。CAM 上用于进入下载模式的 reset 键会复位设备，不是本项目按钮。普通带屏 AtomS3R 的 GPIO41 按钮不能不经确认套用到 CAM。

## 可选：经典 ESP32-WROOM + PN532

以下仅对应 **ESP32-WROOM / DevKitC V4** 与 `esp32dev`，不是 S3/C3 的通用接线。

| PN532 breakout（3.3V 逻辑） | ESP32 |
| --- | --- |
| SCK | GPIO18 |
| MISO | GPIO19 |
| MOSI | GPIO23 |
| SSEL / CS | GPIO27 |
| 3.3Vin | 3V3（确认模块实际标注） |
| GND | GND |

振动驱动逻辑输入改接 GPIO25，外接按钮接 GPIO32–GND，其余驱动电路同上。PN532 需 ISO14443A 4/7 字节 UID 标签，例如 NTAG213。PN532 必须切换 **SPI**；Adafruit breakout 是 SEL0=OFF、SEL1=ON，第三方板以其丝印/手册为准。不要把 5V 逻辑输入 ESP32。

本项目读的是被动 NFC 标签，不支持两个 PN532 主动读卡器互碰、不写 NDEF、不读取支付卡内容、不把 UID 当身份认证。后续双设备互碰需要单独设计 reader/tag-emulation 角色或 BLE 协商。

## 官方依据（核对 2026-09-20）

- [M5 AtomS3R-CAM AI Chatbot：Grove 与底座引脚](https://docs.m5stack.com/en/core/AtomS3R-CAM%20AI%20Chatbot)
- [M5 AtomS3R-CAM：电源、camera 引脚与下载模式](https://docs.m5stack.com/en/core/AtomS3R%20Cam)
- [M5Unified：各主机按钮读取映射](https://github.com/m5stack/M5Unified/blob/master/src/M5Unified.cpp)
- [M5 Unit Vibrator：DIN、供电与 PWM](https://docs.m5stack.com/en/unit/vibrator)
- [Espressif DevKitC V4：引脚与电源](https://documentation.espressif.com/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html)
- [Adafruit PN532 SPI 接线](https://learn.adafruit.com/adafruit-pn532-rfid-nfc/breakout-wiring)
