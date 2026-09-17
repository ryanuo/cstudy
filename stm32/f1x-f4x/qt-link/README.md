# qt-link — F103 → F407 数据监测台（上位机）

Qt 6 + QML + QtShadcn 串口上位机：接收 F407 上送的数据包，**本地重算 CRC-16/MODBUS 校验**后
显示，并把校验结果回给 F407。

```
F103 (USART1 9600) --杜邦线--> F407 USART2 (PA2/PA3 9600)
                                    |
                              F407 USART1 (PA9/PA10)
                                    |
                              外接 USB-TTL (9600 8N1)
                                    |
                                  本程序
```

## 数据协议

**① 二进制帧（推荐，8 字节定长）**

```
FF  D0 D1 D2 D3  CRC_L CRC_H  FE
│   └──── 数据 ───┘  └─ CRC 低/高 ─┘  └ 包尾
└ 包头
```

CRC = **CRC-16/MODBUS**（多项式 0x8005 反射形式 0xA001，初值 0xFFFF），
计算范围是 4 个数据字节，与 F103/F407 两端实现完全一致。

例：`FF 01 02 03 04 A1 2B FE` → 数据 `01 02 03 04`，CRC `0x2BA1`。

**② 文本行（兼容 F407 现有固件的 `USART1_Printf` 输出）**

```
RX: 05 06 07 08  CRC=9825
```

程序两种格式同时支持、自动识别：二进制帧走定长状态机，其余字节按行处理，
非数据文本（`F407 Ready...` / `F407 alive` / `PC cmd: n`）只进原始日志、不计入统计。

## 回复规则

| 情况 | 回复 |
|---|---|
| 校验通过 | `OK\r\n` |
| 校验失败 | `ERR\r\n` |

「已回复」计数只在**真的写出去**时才累加（串口没开不会虚增）。可在界面用开关关闭自动回复。

> ⚠️ 当前 F407 固件的 `USART1_IRQHandler` 只处理 `0xAA` / `0x55`，收到 `OK` 不会有反应。
> 想让 F407 对 `OK` 做动作（比如点灯/计数），需要在 `f407/AGREEMENT/USART.c` 里加分支。

## 构建与运行

```bash
make run      # 首次会配置 + 编译（含 QtShadcn 组件库），之后增量
make build    # 只编译
make clean    # 删除构建目录
make info     # 打印 Qt / 构建目录 / QML 导入路径
```

- Qt 用 **Homebrew 的 `qt`（6.11.2）**：只有它带 QtSerialPort（`~/Qt/6.11.1/macos` 没装）。
- **构建目录默认放本机 `~/dev/qt/qt-link-build`**，不放工程里。原因：本工程在 SMB 网络盘
  （`/Volumes/Keil_v5`）上，CMake 写在该盘上的中间文件会丢，实测编译时报
  `include could not find requested file: build/.qt/appqtlink_qml.cmake`，
  以及 `CMakeFiles/Makefile2` 找不到。要在盘内构建：`make build BUILD_DIR=build`。
- 组件库在 `third_party/qtshadcn`（GitHub 上的 QtShadcn）。
- `main.cpp` 里 `QQuickStyle::setStyle("Basic")` 是**必须**的，否则 macOS 原生样式拒绝自定义
  `background`/`contentItem`，组件会"样式不生效"。

## 离线自测（不用接真机）

`bash tests/run.sh` —— 直接编译工程里的 `SerialLink.cpp` 跑解析用例（二进制帧 / CRC 错 /
分包到达 / 文本行 / 假包头重同步 / 混合流 / 回复计数等，共 25 项）。

界面上原有一栏「链路自检」（自测·正确包 / 自测·错误包 / 发送 0xAA / 发送 0x55）已按需求移除。
需要时把按钮加回「自动回复 · 手动发送」那一行即可，底层接口都还在：

```qml
ShadcnButton { text: "自测·正确包"; onClicked: serialLink.injectHex("FF 01 02 03 04 A1 2B FE") }
ShadcnButton { text: "自测·错误包"; onClicked: serialLink.injectHex("FF 01 02 03 04 00 00 FE") }
ShadcnButton { text: "发送 0xAA";   onClicked: serialLink.sendHex("AA") }  // F407 固件回 CC
ShadcnButton { text: "发送 0x55";   onClicked: serialLink.sendHex("55") }  // F407 固件回 DD
```

## 界面

- 窗口初始尺寸按**屏幕可用区域**自适应（1440×900 的屏上约 1180×745，不会超出菜单栏/Dock）
- 顶部：串口选择（CH340 等 USB 串口优先）、刷新、波特率（默认 9600）、连接/断开、连接状态
- 自动回复开关 + 手动发送 + 清空统计
- 统计：收到总数 / 校验通过 / 校验失败 / 已回复
- 左：数据表（列 `# / 时间 / PAYLOAD (HEX) / CRC / 校验`，D0~D3 合成一列；**列宽按容器宽度按比例分配**、数值列 `align: "right"`；最新在上，默认保留 300 条），点行看详情；无数据时显示空状态提示
- 右：本包详情（**卡片外观 + 与表格同一种描边 `theme.border`**，Label 固定宽 76 + 左对齐成两列栅格；`数据` 加大加粗、时间/CRC 等宽、`原始报文` 用浅灰代码块容器；**按内容高度自适应、不拉满**）
- 状态表达：表格「校验」列正常态只显示一个小 `✓`、异常态才是 `✗ 失败`（弱化正常、突出异常）；详情顶部胶囊 通过=浅绿底深绿字、失败=浅红底深红字
- 字体：`main.cpp` 里应用默认字体族把 `Menlo` 排第一位 → 时间戳/十六进制/CRC 全局等宽（**表格单元格字体在库里写死 `font.pixelSize: 14`，只能靠应用默认字体族改**），中文无等宽字形自动回落 `PingFang SC`
- 底部：原始日志条（**整宽 · 固定 3 行高**，标题行压到 18px，可选中复制，接收/校验/回复）
- 三种大容器（表格 / 详情卡 / 日志条）边框统一用 `theme.border`，避免"表格有框、卡片没框"的割裂感
- 右上角可切换深浅色主题（QtShadcn token 全局随动）

### 调试用截图（可选）

`QTLINK_GRAB=1 QTSHADCN_SCREENSHOT=1` 启动会等 2.6s 把窗口渲染结果存到 `/tmp/qtlink_shot.png`（离屏软件渲染下必须开 `QTSHADCN_SCREENSHOT=1`，否则 ShadcnCard 的 GPU 阴影层不上屏、卡片是空白的）。
