# Music Magician · NFC Haptic MVP

**把一次碰触，变成一段可以自己写的振动。**

A small, local, open-source haptic controller. Edit three pulse patterns in JSON;
trigger them with an NFC tag, an external button, a USB serial command, or an
explicitly armed onboard accelerometer shake detector on AtomS3R-CAM.
No account, API key, cloud service, contact list, microphone, or camera access.

这是通用的「输入 → 自定义振动」代码切片。它不是音乐生成系统，也不包含产品外观、私人研究或未公开的感知算法。软件通过编译/桌面测试不等于实物已振动。

## 今晚可以怎么跑

| 手上有什么 | 路径 | 还缺什么 |
| --- | --- | --- |
| 只有电脑 | 跑下面的桌面模拟器，共用固件控制核心 | 无；只显示 PWM 时间线，不会产生物理振动 |
| M5Stack AtomS3R-CAM AI Chatbot | 默认 `m5atoms3r_cam`；USB 串口 `play hello` → Grove G1 输出 | 独立驱动 + ERM 马达；可选 G2 外接按钮 |
| 只有 AtomS3R-CAM，无 NFC/按钮 | 串口 `arm`，静置校准，再摇动机身 → 已有振动模式 | 输入只用板载 BMI270；实际振动仍需外接驱动/马达 |
| AtomS3R-CAM + M5 Unit Vibrator U059 | `m5atoms3r_cam_u059`；Grove 黄线 G2、10 kHz PWM | U059 模块、足够的 5V 供电；串口触发即可 |
| 经典 ESP32-WROOM / DevKitC V4 | `esp32dev`；PN532 标签 → GPIO25 输出 | PN532、NFC 标签、马达及驱动；GPIO32 外接按钮可先替代 NFC |
| Seeed reSpeaker，具体型号未核 | 先保留原有语音固件 | 核对型号与引脚后再适配；本仓库不声称该板支持 |

AtomS3R-CAM AI Chatbot 的官方组合是 CAM 主机 + Atomic Voice Base，没有内置振动马达/NFC；CAM 上的 reset/download 键不能代替本项目的外接按钮。请先读 [接线与物料](docs/WIRING.md)。

### 1. 无硬件演示（macOS / Linux）

需要 Python 3.10+ 和一个 C++17 编译器；无需安装 Python 第三方库。

```sh
bash scripts/build_desktop.sh
./build/simulator < simulator/demo.txt
./build/simulator
```

交互示例：

```text
tag 12345678
advance 1500
tag 12345678
remove
advance 600
tag DEADBEEF
advance 100
stop
```

`advance` 推进的是模拟时间。模拟器用 **同一份 C++ Player、PresenceGate 和生成后的配置**，不访问串口、不模拟传感器噪声/电机惯性。测试含地址与未定义行为检查、时间回绕、冷却、停止、标签长时间停留、按钮去抖和非法配置。

### 2. 编译固件

```sh
python3 -m venv .venv
.venv/bin/python -m pip install platformio==6.1.18
.venv/bin/pio run -e m5atoms3r_cam
# 使用 M5 Unit Vibrator U059 的可选配置：
.venv/bin/pio run -e m5atoms3r_cam_u059
# 经典 ESP32 + PN532 可选：
.venv/bin/pio run -e esp32dev
```

PlatformIO 首次会下载编译器和 Arduino 框架。固件锁定 `espressif32@6.5.0` / Arduino-ESP32 `2.0.14`，这是已完成本地验证的组合；配置生成在每次构建前自动执行。

**上传会替换当前板上的应用固件。** 先确认型号、原固件恢复方式与接线，记录该板原有配置；本项目没有自动探测并刷写脚本。下面命令由操作者将占位串口替换为已确认的设备后执行：

```sh
.venv/bin/pio device list
.venv/bin/pio run -e m5atoms3r_cam -t upload --upload-port YOUR_CONFIRMED_PORT
.venv/bin/pio device monitor -b 115200 --port YOUR_CONFIRMED_PORT
```

串口命令需要换行：

```text
list
play hello
play heartbeat
play celebrate
stop
```

`esp32dev` 支持 `learn`，在本地串口只打印下一张读到的标签 UID 一次。默认日志不打印 UID；不会写标签。PN532 不在时，经典 ESP32 环境仍可使用按钮/串口，接回读卡器后复位重新初始化。

### 不加 NFC/按钮：直接用 CAM 机身摇动

两个 CAM 固件配置都包含可选 IMU 功能，**上电默认关闭**。使用官方 M5Unified 驱动访问板载 BMI270（SDA GPIO45、SCL GPIO0）；Grove GPIO1/2 留给振动输出/外接按钮。传感器输入不需要新增配件，触觉输出仍需要马达。

```text
arm
```

把设备放稳，等待串口从 `motion calibrating` 变为 `motion armed`，通常需要至少 2 秒。随后做一次明显来回摇动；两次分离的加速度峰值会播放 `button_pattern` 指定的模式，并输出 `imu-shake hello started`（名称随配置改变）。输入 `disarm` 或 `stop` 会停止播放并关闭动作检测。再次启动必须重新 `arm` 和校准。

这是**阈值摇动检测**，不是 AI 手势识别、打响指识别或 head tracking。阈值是待实物校准的起点，不能据此声称日常携带零误触发。静置校准超时、采样失败/超时、异常值或时间倒退都会关闭检测；马达播放期间暂停 I²C 读取，结束后必须重新观察到安静状态，避免马达震动不断触发自己。具体规则与验收见 [IMU 说明](docs/MOTION.md)。

无需接板的合成样本演示（不会访问真实 IMU）：

```sh
bash scripts/build_desktop.sh
./build/simulator < simulator/motion-demo.txt
```

模拟器也支持 `arm`、`still 2020`、`imu 0 0 2 20`（x/y/z 单位 g，最后为推进的毫秒数）、`disarm`，可直接注入样本。

### 3. 自定义一种触感

编辑 [config/patterns.json](config/patterns.json)，每个步骤为 `[持续毫秒, PWM 强度]`，强度 `0` 表示停顿。例如：

```json
{"name": "hello", "steps": [[90, 180], [100, 0], [90, 180]]}
```

这是「短—停—短」。PWM 不是主观感知强度的线性标尺；不同马达/电压会有启动阈值，需要接实物校准。保存后重新编译/上传，桌面则重新运行构建脚本。

- `button_pattern`：按钮要播放的名称。
- `unknown_tag_pattern`：未配置的标签默认播放 `hello`；设为 `null` 可忽略未知标签。
- `tags`：UID → pattern。仓库内 UID 都是合成示例，与真实用户无关。
- `cooldown_ms`：播放结束后的冷却，默认 1000 ms。
- `release_ms`：连续观察到无卡后才能重新触发，默认 500 ms。

生成器限制每段 1–500 ms，强度 0–230，单模式总时长 ≤ 3 s、累计通电 ≤ 1.5 s；固件核心也校验这些上限。播放中拒绝叠加；播放中（含停顿）暂停 NFC 轮询；长时间把卡放在读卡器上只触发一次。按钮消抖 30 ms。`stop` 即刻让 PWM 归零并进入冷却。

## 验证边界

当前记录在 [verification/STATUS.md](verification/STATUS.md)。未连接板卡时，固件、PN532 通信、物理马达、电源稳定性和实际振动强度均不能算硬件实测。下次接板按 [硬件验收](docs/HARDWARE_ACCEPTANCE.md) 执行。

三个固件配置已在本地编译成功。云端 CI **未执行**：发布时现有 GitHub 登录不具备写入 workflow 的 scope，工作流保存在 [ci/check.yml.template](ci/check.yml.template)。有合适权限的维护者可复制至 `.github/workflows/check.yml` 启用；本项目不要求新增密钥。

## 开源范围与来源

原创代码为 [MIT](LICENSE)，第三方依赖及官方引脚资料见 [NOTICE](NOTICE.md)。本仓库实现已有公开先例的通用 NFC/按钮触发振动功能，不主张此组合属于专利首创；开源许可证不代表专利授权书或自由实施（FTO）结论，也不授予任何第三方专利或商标权。使用限制见 [SECURITY](SECURITY.md)。
