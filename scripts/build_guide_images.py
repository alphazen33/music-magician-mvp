#!/usr/bin/env python3
"""Rebuild the six original, editable installation diagrams. Python stdlib only."""
from pathlib import Path
from html import escape
OUT = Path(__file__).resolve().parents[1] / 'docs/images'
OUT.mkdir(parents=True, exist_ok=True)
INK='#163d37'; GREEN='#246e58'; MUTED='#61766f'; BG='#f7f5ee'; ORANGE='#bd542e'

def text(x,y,s,size=24,color=INK):
    return f'<text x="{x}" y="{y}" font-size="{size}" fill="{color}">{escape(s)}</text>'
def rect(x,y,w,h,color='#ffffff',r=20):
    return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="{color}"/>'
def line(x,y,xx,yy,color=GREEN,width=4):
    return f'<path d="M{x} {y} L{xx} {yy}" stroke="{color}" stroke-width="{width}" fill="none"/>'
def card(x,y,w,title,lines):
    return rect(x,y,w,180)+text(x+24,y+42,title,27)+''.join(text(x+24,y+82+i*32,s,21,MUTED) for i,s in enumerate(lines))
def page(num,name,title,sub,body):
    svg=f'''<svg xmlns="http://www.w3.org/2000/svg" width="1440" height="900" viewBox="0 0 1440 900"><title>{escape(title)}</title><desc>{escape(sub)}</desc><g font-family="PingFang SC, Noto Sans SC, sans-serif">{rect(0,0,1440,900,BG,0)}{text(64,60,'MUSIC MAGICIAN / 安装图解',20,GREEN)}{text(64,140,title,48)}{text(64,190,sub,24,MUTED)}{body}{line(64,826,1376,826,'#d9ded4',2)}{text(64,866,'代码与编译已验证 · 硬件尚未实测 · 接线按信号名称核对，示意图不代表实物比例',19,MUTED)}{text(1280,866,f'{num:02d} / 06',20,GREEN)}</g></svg>'''
    (OUT/name).write_text(svg,encoding='utf-8')

b=card(64,245,636,'AtomS3R-CAM / 已有两种配置',['m5atoms3r_cam：G1 → 独立驱动','m5atoms3r_cam_u059：G2 → U059','两种配置均有串口、按钮、可启用的摇动检测'])
b+=card(724,245,652,'经典 ESP32-WROOM / 已有配置',['esp32dev：PN532 → 标签触发','串口、外接按钮 → G25 振动输出','当前配置没有板载 IMU 摇动功能'])
b+=card(64,450,636,'其他 S3 / C3 / C6 / S2 / …',['不能直接套用现有固件或接线','需核对芯片、Flash、PSRAM、USB、GPIO','传感器与 Arduino / PlatformIO 支持也需适配'])
b+=card(724,450,652,'Seeed reSpeaker / 待明确型号',['同一个产品家族可能采用不同主控','尚未适配，不建议直接刷入本仓库固件','原有语音与 Chatbot 程序不在本次功能内'])
b+=rect(64,660,1312,104,'#e4eddf')+text(90,705,'可移植的是控制逻辑；固件、引脚和传感器适配要按具体开发板选择。',30)+text(90,740,'“本地编译通过”尚不代表“每块板都已连接测试”。',22,MUTED)
page(1,'01-compatibility.svg','不是所有 ESP32 都能直接刷','先确认完整板名，再选择环境；esp32dev 不是整个 ESP32 家族的通用配置。',b)

b=card(64,245,410,'01 / 你已有的主机',['AtomS3R-CAM AI Chatbot','本项目只使用 CAM 控制器','相机、语音与联网未启用'])
b+=card(494,245,422,'02 / 增加振动输出',['M5 Unit Vibrator U059','内置马达与 MOSFET 驱动','配套 Grove HY2.0 4P 线'])
b+=card(936,245,440,'03 / 电脑与连接',['USB-C 数据线 + 电脑','Python + PlatformIO','核对 5V 供电能力'])
b+=text(84,510,'电脑',36)+line(205,500,470,500)+text(270,480,'USB 数据 / 供电',19)+text(500,510,'CAM',36)+line(625,500,975,500)+text(720,480,'Grove 4P',19)+text(1020,510,'U059',36)
b+=rect(64,566,1312,199,'#e4eddf')+text(90,615,'这是最省接线的现有方案：不需要 NFC、不需要额外按钮。',30)+text(90,661,'先用串口 play hello 验证输出，再用 arm 启用板载摇动输入。',25)+text(90,708,'U059 官方 50% PWM 工况：5V / 424.35mA；这不是整机峰值或电源选型上限。',23,MUTED)
page(2,'02-parts.svg','先搭出最短的一次振动','推荐路线：你的 AtomS3R-CAM + U059；不需要焊接裸马达驱动电路。',b)

b=rect(64,250,390,445)+rect(986,250,390,445)+text(94,302,'AtomS3R-CAM',32)+text(1016,302,'U059',32)
for y,left,right,col in [(370,'GND / 黑','黑 / GND','#26352f'),(445,'5V / 红','红 / 5V','#bd542e'),(520,'GPIO2 / 黄','黄 / DIN','#ac8614'),(595,'GPIO1 / 白','白 / NC','#8f9e98')]:
    b+=text(94,y,left,27)+line(425,y-8,1005,y-8,col,6)+text(1040,y,right,26)
b+=text(540,500,'G2 输出 PWM · 10 kHz',25)+text(534,637,'白线在 U059 端不连接电路',21,MUTED)
b+=text(88,745,'固件环境必须选 m5atoms3r_cam_u059；默认 CAM 环境把振动输出放在 G1。',26,ORANGE)
page(3,'03-u059-wiring.svg','一根线，四个信号','断电连接，核对线色与接口定义；此图是电气对应关系，不表示插头观察方向。',b)

b=card(64,235,410,'CAM + 独立 ERM 驱动',['环境：m5atoms3r_cam','GPIO1 → 驱动逻辑输入','可选按钮：GPIO2 ↔ GND'])
b+=card(494,235,422,'经典 ESP32 + 独立驱动',['环境：esp32dev','GPIO25 → 驱动逻辑输入','可选按钮：GPIO32 ↔ GND'])
b+=card(936,235,440,'3V ERM 的电源路径',['稳压 3V → 驱动 / 马达电源','驱动必须兼容 3.3V 逻辑','电源地、驱动地、主控地共地'])
b+=rect(64,445,810,325)+text(88,485,'经典 ESP32-WROOM → PN532（SPI）',29)
for i,(a,c) in enumerate([('GPIO18 → SCK','GPIO19 → MISO'),('GPIO23 → MOSI','GPIO27 → SSEL / CS'),('3V3 → 3.3Vin*','GND → GND')]):
    b+=text(88,540+i*50,a,25)+text(475,540+i*50,c,25)
b+=text(88,733,'* 仅用于支持此供电与 3.3V 逻辑的模块；按模块手册切换 SPI。',19,MUTED)
b+=rect(896,445,480,325,'#f3e2d5')+text(920,485,'三个接线边界',29,ORANGE)
for i,s in enumerate(['马达不能直接接 GPIO','5V 不能直接供给 3V 马达','此驱动路径不支持 LRA','完整 MOSFET 电路见 WIRING.md']):
    b+=text(920,545+i*49,s,23)
page(4,'04-alternative-wiring.svg','需要 NFC 时，再加读卡器','这是经典 ESP32-WROOM 的接线；不能直接套到 AtomS3R-CAM、C3 或其他 S3。',b)

b=''
items=[('01 / 确认与准备','确定板名和振动模块','记录原固件恢复方式'),('02 / 安装工具','克隆仓库，安装 PlatformIO','第一次编译需联网下载依赖'),('03 / 选择环境','U059 选 CAM 的 U059 环境','构建成功，再进行烧录'),('04 / 找到串口','pio device list','选择你已确认的设备端口'),('05 / 烧录程序','pio run … -t upload','会替换当前 Chatbot 应用'),('06 / 打开串口','115200 波特率；发送换行','list → play hello → stop')]
for i,(a,c,d) in enumerate(items):
    x=64+(i%3)*444; y=245+(i//3)*236
    b+=card(x,y,424,a,[c,d])
b+=text(88,754,'完整可复制命令见配套文档。当前没有自动选板、网页一键烧录或手机配置应用。',25,MUTED)
page(5,'05-install.svg','安装是六步，还不是一键','接线简单的 U059 路线，也需要一次开发工具安装、编译和上传。',b)

b=card(64,245,410,'先确认输出',['串口发送：play hello','马达应播放“短—停—短”','stop：PWM 归零，动作检测关闭'])
b+=card(494,245,422,'再启用摇动',['发送 arm，放稳至少 2 秒','等到 motion armed，再来回摇动','双峰触发；不是打响指识别'])
b+=card(936,245,440,'最后写自己的触感',['编辑 config/patterns.json','每段：[毫秒, PWM 数值]','保存 → 重新编译 → 重新烧录'])
b+=text(88,495,'hello',34)+text(250,495,'[[90, 180], [100, 0], [90, 180]]',30)
b+=rect(250,550,270,105,GREEN,8)+line(520,655,820,655,MUTED,3)+rect(820,550,270,105,GREEN,8)
b+=text(315,600,'PWM 180',25,'#ffffff')+text(575,610,'PWM 0',25)+text(885,600,'PWM 180',25,'#ffffff')
b+=text(320,694,'90 ms',25)+text(615,694,'100 ms',25)+text(890,694,'90 ms',25)
b+=text(88,758,'总时长 280 ms；PWM 不是实际体感强度。电机有惯性，指令波形不等于物理振动波形。',24,MUTED)
page(6,'06-first-use.svg','把一句“你好”，写成触感','先验证串口，再验证动作；拔电或 stop / disarm 后，需要重新 arm 才能摇动触发。',b)
print(f'Wrote 6 SVG diagrams to {OUT}')
