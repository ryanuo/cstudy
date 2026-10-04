# STM32F407 物联网控制节点（OneNET Studio + ESP8266 + Web 面板 + 语音控制）

STM32F407ZGT6 通过 ESP8266（AT 指令）接入 **OneNET Studio 物模型**，双向通信：周期上报温湿度与执行器状态、
接收属性下发（开关灯/蜂鸣器/风扇）与**物模型服务调用**（重启）。配套 `web/` 是一个浏览器控制面板
（纯静态页 + serverless Python API，可部署到 Vercel），支持网页控制、历史曲线、设备重启，以及
**语音控制**（浏览器语音识别 → 大模型解析意图 → 下发 → 语音播报）。

```
         ┌──────────────┐  AT/UART4   ┌─────────┐   MQTT(1883)   ┌──────────────┐
         │ STM32F407ZGT6│◄───────────►│ ESP8266 │◄──────────────►│ OneNET Studio│
         │  DHT11/3路灯  │             └─────────┘                │   物模型      │
         │ 蜂鸣器/风扇   │                                        └──────┬───────┘
         └──────┬───────┘                                               │ REST
                │ printf/UART5（PC12/PD2，CH340 看日志）                │
                │                                        ┌──────────────┴──────────────┐
                └── LCD 本地状态显示                       │  Web 面板（Vercel）           │
                                                          │  静态页 + /api/*.py 函数      │
                                                          │  口令门 · 限流 · 语音控制      │
                                                          └──────────────────────────────┘
```

## 功能一览

| 功能 | 说明 |
|---|---|
| 三路 LED / 蜂鸣器 / 风扇 | 网页、语音、OneNET 平台三处都能控；上报带执行器状态，点击后能回读真值 |
| 温湿度采集 | DHT11，采样 2s 节流、上报周期 5s（`APP_DHT11_PERIOD_MS` / `APP_REPORT_PERIOD_MS`） |
| 物模型服务调用 | `reboot`：面板按钮两段式确认 → 设备先应答再复位，自动重连并恢复上报 |
| Web 面板 | 口令门 + 限流、乐观更新 + 回读、历史曲线、离线置灰、重启按钮 |
| 语音控制 | 「太热了」→ 开风扇并播报温度；「把灯都关了吧」→ 一次下发多路；追问自动续听 |
| 朗读 | 豆包语音合成大模型 2.0（音色可在面板切换）优先，**失败静默回退浏览器朗读** |
| 本地显示 | LCD 显示启动/联网/上报状态（`app_ui.c`） |

## 硬件

| 部件 | 接线（当前代码取值） | 备注 |
|---|---|---|
| ESP8266 | **UART4**（`BSP_ESP8266_Init(&huart4)`） | AT 指令，115200 |
| printf 日志 | **UART5 = PC12(TX)/PD2(RX)** | CH340 USB-TTL，`make serial` |
| LED ×3 | PE3 / PE4 / PG9 | **低电平点亮**（`BOARD_LED_ON_LEVEL 0`），分端口宏在 `board_pins.h` |
| 蜂鸣器 | PG7 | 高电平响 |
| 风扇 | PA5 + PA7（两引脚） | 高电平开 |
| DHT11 | CubeMX 标签 `DHT11_DATA` | 采样节流在 `bsp_dht11.c` |
| LCD | 见 `BSP/Src/lcd.c` / `oled.c` | `app_ui.c` 用它显示状态 |

> **引脚唯一真值源是 `BSP/Inc/board_pins.h`**；外设时钟/GPIO 初始化由 CubeMX 生成（`Core/`），
> 驱动文件里不写初始化 —— 改引脚请改 `.ioc` 与 `board_pins.h`，别在驱动里散落魔数。

## 目录结构

```
stm32mx407test_xinxin/
├── APP/                     业务层
│   ├── Inc/  app_config.h   唯一配置：WiFi / OneNET 凭据 / 周期 / 调试开关
│   │         app_device.h   物模型映射、下行处理表、上报
│   │         app_onenet.h   通用 MQTT/AT 层接口
│   │         app_ui.h        LCD 显示
│   └── Src/  app_device.c   ★ 全工程唯一使用 cJSON 的模块（解析/拼装物模型报文）
│             app_onenet.c   ★ 只管 MQTT/AT（不 include cJSON/lcd/dht11）
│             app_wifi.c     ESP8266 联网流程
│             app_ui.c       LCD 状态页
├── BSP/                     板级驱动（led / beep / fan / bsp_dht11 / bsp_esp8266 / lcd / oled / usart）
│   └── Inc/board_pins.h     ★ 引脚与有效电平的唯一真值源
├── Libs/cJSON/              cJSON（`CJSON_NESTING_LIMIT=8`，解析栈用量实测每层 ≈104B）
├── Core/                    CubeMX 生成（main.c 的 USER CODE 区放启动流程）
├── Drivers/                 HAL / CMSIS
├── web/                     Web 面板 + serverless API（Vercel Root Directory）
├── Makefile                 包装 CMake/Ninja + 烧录 + 串口 + 面板快捷命令
└── CMakeLists.txt / CMakePresets.json / STM32F407xx_FLASH.ld
```

分层规矩（改代码前先看这条）：

- `app_onenet.c` 是**通用层**：只做 MQTT/AT，不 include `cJSON.h` / `lcd.h` / `bsp_dht11.h`；
- `app_device.c` 是**业务层**：物模型映射、下行分派、JSON 拼装全在这里；
- 引脚/有效电平只在 `board_pins.h`，配置只在 `app_config.h`。

## 固件构建与烧录

工具链：`arm-none-eabi-gcc`（本机在 `~/dev/toolchains/bin`，Makefile 已加进 PATH）。

```bash
make            # = make build：cmake --preset Debug + 构建
make release    # Release(-O3) 构建
make size       # 看 Flash/RAM 占用
make bin / hex  # 生成 .bin / .hex
make flash      # 编译 + 生成 bin/hex + 烧录（默认 openocd + ST-Link）
make reset      # 复位运行
make openocd    # 只开 openocd 服务（调试用）
make serial     # 打开串口看 printf（默认 /dev/cu.wchusbserial1110 @115200）
make clean / distclean / rebuild
```

关键点：

- **必须走 `cmake --preset Debug|Release`**：工具链文件写在 `CMakePresets.json` 里；手工 `cmake -S . -B build`
  会拿宿主编译器（AppleClang）配置，且 configure 还会"成功"，到 build 才炸。
- preset 每次都重新 configure，顺带解决 `file(GLOB BSP/Src/*.c)` 的坑（新增源文件不重配就不会被编进去）。
- `Makefile` 的**变量行不能写行内注释**（`#` 前的空格会并进变量值，导致 `objcopy` 用法报错）。
- 构建目录建议放本机（项目在共享盘/SMB 上时，生成文件会莫名丢失）。

## 物模型（OneNET Studio）

| 标识符 | 类型 | 取值 |
|---|---|---|
| `led1` / `led2` / `led3` | string(8) | `"on"` / `"off"`（**小写**，与物模型完全一致） |
| `buzzer` / `fan` | bool | `true` / `false` |
| `temperature` / `humidity` | number | DHT11 读数 |
| 服务 `reboot` | — | 面板按钮触发；设备先回应答再复位 |

下行协议要点（都是实测出来的）：

- **属性下发**：`$sys/{pid}/{did}/thing/property/set` → 应答 topic 是 `<下发topic>_reply`（即 `…/set_reply`），
  报文 `{"id":"…","code":200,"msg":"success"}`。
- **服务调用**：下发 topic `…/thing/service/{identifier}/invoke`；**订阅必须写到 `/invoke` 这一层**
  （`/thing/service/+/invoke`，少一层平台就报 `dev not subscribed`）；
- **应答 topic = 下发 topic + `_reply`**（`…/invoke_reply`，与 `set → set_reply` 同规矩）；
- **应答体必须带 `data`**（`{"id":"…","code":200,"msg":"success","data":{}}`），缺了平台报 `response invalid`。
- 上报报文要**按属性名包装**：`{"led1":{"value":"on"}}`，平铺会回 `2402 request format error`。
- 常见平台错误码：`10411` identifier not exist（物模型没这个标识符）、`2402` 报文格式错、
  `10415` 未订阅 / 标识符不存在 / 应答非法。

## Web 面板（`web/`）

`web/` 就是 Vercel 的 Root Directory —— 面板、API、测试、本地服务器全在这里，固件目录保持干净。

```
web/
├── index.html / style.css / app.js    面板（Vue3 CDN，读属性 / 下发 / 曲线 / 重启）
├── config.js                          同源 API 封装 + 口令输入条（token 不在前端）
├── voice.js                           语音层：识别 → 意图 → 执行 → 播报
├── api/_lib.py                        纯逻辑 + 出网调用（不 import flask，便于单测）
├── api/{health,chat,onenet,tts}.py    一文件一端点，Flask WSGI 导出 app
├── tests/                             pytest + node --test
├── tools/dev_server.py                本地三合一服务（不需要 vercel 登录）
├── requirements.txt / vercel.json / .vercelignore / .env.example
└── .env.local                         本地凭据（**已 gitignore**）
```

端点：

| 端点 | 作用 |
|---|---|
| `POST /api/onenet` | `{op, params, body}`；op 白名单：`deviceDetail`/`getProperty`/`getHistory`/`setProperty`/`callService`。设备 token 只在后端 |
| `POST /api/chat` | `{text, controls, cards, history, pending}` → 严格 JSON 意图（动作走服务端白名单校验） |
| `POST /api/tts` | `{text, rate, pitch, volume, voice}` → `audio/mpeg`（音色走白名单） |
| `GET /api/health` | 模型名 / 限流是否就绪 / 朗读通道与音色清单 |

本地起：

```bash
make web-install     # 建工程根 .venv 装依赖（flask / requests）
make web             # 起面板 + API：http://127.0.0.1:3000
make web-test        # pytest + node --test
make web-open        # 打开页面
# NOPASS=1 跳过口令（仅本地）；WEB_PORT=3001 换端口
```

部署：Vercel 里 Root Directory 选 `web/`，把 `.env.example` 里的变量加到环境变量
（`DASHSCOPE_API_KEY` / `ONENET_TOKEN` / `ONENET_PRODUCT_ID` / `ONENET_DEVICE_NAME` /
`PANEL_PASSWORD` / `UPSTASH_REDIS_REST_URL`+`TOKEN`（serverless 限流必须外部存储）/
`VOLC_TTS_API_KEY`）。改环境变量后要 **Redeploy** 才生效。

## 语音控制链路

```
浏览器识别                      /api/chat                面板桥(window.__panel)        朗读
webkitSpeechRecognition  ──►  qwen-flash 出严格 JSON  ──►  复用面板同一套下发逻辑  ──►  云端 TTS
（免费、无后端）               动作/标识符服务端白名单      乐观更新 + 回读真值          失败回退浏览器朗读
```

- 模型：`QWEN_MODEL=qwen-flash`（实测 0.5s/句、11/11 正确；3.x 是思考型，慢 4~17 倍，用 `QWEN_THINKING=0` 关掉）。
- 意图契约：`{action, target, targets, value, reply}`，`action ∈ toggle|toggle_many|query|refresh|reboot|chat|unknown|steps`；
  「太热了」这类感受用 `steps`（开风扇 + 报温度）；一次多路用 `toggle_many`（**合成一条下发**，ESP8266 背靠背发会 ERROR）。
- 多轮追问：模型反问时前端**念完自动重新开麦**，并把上一轮缺的槽位（`pending`）结构化回传。
- 破坏性动作（重启）语音只把按钮推到确认态，**不直接执行**。
- 朗读优先走云端（豆包语音合成大模型 2.0，音色在面板「云端」下拉框里切：Vivi / 小何 / 云舟 / 小天 2.0），
  任何一步失败都**静默回退**浏览器 `speechSynthesis`；点「试听」才会把失败原因显示在提示行。

## 配置与凭据（⚠️ 请勿提交）

| 位置 | 内容 |
|---|---|
| `APP/Inc/app_config.h` | 固件侧：WiFi SSID/密码、OneNET product_id/device_id/token、上报周期、`APP_ONENET_DEBUG` |
| `web/.env.local`（本地）/ Vercel 环境变量 | 面板侧：DashScope key、OneNET token、面板口令、Upstash、火山 TTS API Key |

> **不要把这些值提交到仓库。** 本仓库是公开仓库：`app_config.h` 里若有真实凭据，等于公开泄露
> （WiFi 密码、设备 token 都可被直接使用）。推荐做法：把凭据挪到一个被 `.gitignore` 忽略的头文件
> （例如 `APP/Inc/app_secrets.h`），`app_config.h` 用 `#if __has_include("app_secrets.h")` 引入、缺省给占位值；
> 面板侧一律走环境变量。已经提交过的凭据请到对应控制台**重新签发/改密**（历史里的删不掉）。

## 测试

```bash
make web-test        # pytest（_lib 纯逻辑 + 意图校验 + TTS 报文/流式解析）+ node --test（前端接线）
```

- 后端单测不依赖 flask：`_lib.py` 不 import flask，出网调用在测试里被打桩。
- 前端用最小 DOM 桩 eval `config.js`/`voice.js`，断言「点击 → 识别 → 意图 → 执行 → 播报」整条链，
  比开无头浏览器快且稳（无头 Chrome + CDP 容易卡在端口/目标列表上）。
- 固件侧改动建议至少跑一次主机侧单测（解析/拼装报文不依赖硬件）。

## 已知坑（踩过的）

- ESP8266 背靠背发 `MQTTPUBRAW` 会回 `ERROR`：多路下发**合成一条**属性报文。
- 属性上报要带 `{"value":...}` 包装，否则平台 `2402`。
- 物模型标识符大小写敏感：`led1` ≠ `LED1`（`10411`）。
- 火山 TTS：这条 HTTP Chunked 接口**成功码是 `20000000`**（文档另一页写 `0`，实测以真机为准）；
  长文本异步接口（`/api/v3/tts/submit`）文档自述非实时、排队数十分钟，**不能**用于面板即时朗读。
- 面板浮层显隐别用 class 判断（淡入是下一帧才加 `is-show`，快速开停会卡住），用独立状态位。
- Electron/内嵌 webview 里没有 `window.prompt`，口令用页面内输入条。
- 换环境变量记得 Redeploy；给本地调试留的"跳过口令"开关**不要**带到线上。
