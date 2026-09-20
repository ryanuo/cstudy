# USER/back —— 旧版本留档

这里放**改动前**的旧文件。项目 `.gitignore` 里写了 `*.bak`，所以这些文件**只存在本机、不进仓库**。

- 想用某一版：`cp main.c.wifi-web2.bak ../main.c`，然后回 Keil 重新 Rebuild + Download
- **2026-09-20 整理**：模块类备份（`esp8266.c/h.*`、`web.c.html版`、`web.h.html版`、`OLED.c.bak`）已清掉
  —— 它们在 git 历史里都能取回（见文末），只保留 main 相关的

## 一、ESP8266 WiFi + 手机网页 那一轮（9-19）

| 文件 | 行数 | 时间 | 是什么 |
|---|---|---|---|
| `main.c.esp.bak` | 48 | 09-19 15:29 | 第一次接 ESP8266：只会 AT 收发，用自写假延时、失败就死等 |
| `main.c.diag1.bak` | 157 | 09-19 15:41 | 诊断固件 v2：OLED 显示收到的原始字节（hex）+ 长度，用来分清「模块不说话」还是「我们收不到」 |
| `main.c.diag3.bak` | 189 | 09-19 15:58 | 诊断固件 v3：加 SELF（片内回环自测）/ PIN（读引脚电平）/ BOOT（74880 抓模块启动信息） |
| `main.c.esp-fixed.bak` | 171 | 09-19 15:37 | 修好延时与重试后的正式 WiFi 固件（AT → CWMODE → CWJAP → CIFSR） |
| `main.c.wifi-v1.bak` | 209 | 09-19 16:02 | 连 WiFi 迭代 1 |
| `main.c.wifi-v2.bak` | 215 | 09-19 16:05 | 迭代 2：超时改 SysTick 真实毫秒、接收改两级缓冲 |
| `main.c.wifi-v3.bak` | 221 | 09-19 16:07 | 迭代 3：加 AT+CWLAP 扫描 + ATE0 关回显 |
| `main.c.wifi-v4.bak` | 248 | 09-19 16:11 | 迭代 4：用扫描到的名字去连（SSID 大小写对齐） |
| `main.c.wifi-v5.bak` | 292 | 09-19 16:13 | 迭代 5：内容最多的一版（293 行，诊断代码都还在） |
| `main.c.wifi-v6.bak` | 187 | 09-19 16:14 | 清掉诊断代码后的精简版（188 行） |
| `main.c.wifi-web.bak` | 189 | 09-19 16:19 | 修好 no ip 之后（ESP8266_FindIp 抓 STAIP），手机网页还没加进来 |
| `main.c.wifi-web2.bak` | 354 | 09-19 16:36 | 手机网页控制那一版：CIPSERVER + 手拼 HTTP + 光敏/电位器 JSON（355 行，模块化前最后的大 main） |

## 二、各外设实验（9-11 ~ 9-18）

| 文件 | 行数 | 时间 | 是什么 |
|---|---|---|---|
| `main.c.bak` | 121 | 09-11 10:36 | 9-11 最早的 F407 打底：OLED + 光敏读数 + LED + 蜂鸣器（TIM6） |
| `main.c.diag.bak` | 151 | 09-11 17:38 | 同上的调试中间态 |
| `main.c.Encoder.bak` | 158 | 09-11 17:47 | 编码器计数实验（CountSensor） |
| `main.c.timer.bak` | 18 | 09-12 14:43 | TIM2 定时器中断实验（**UTF-8**） |
| `main.c.etim.bak` | 18 | 09-12 15:08 | TIM2 编码器模式实验（**UTF-8**） |
| `main.c1.f103-link.bak` | 119 | 09-17 14:20 | 双机链路：F407 收 F103 的 CRC 包 → 转发上位机 → OLED 显示 D0~D3/计数/CRC（**UTF-8**） |
| `main.c2.dac-out.bak` | 33 | 09-17 16:44 | DAC 固定输出草稿（DAC_SetChannel1Data(2048) 循环） |
| `main.c3.light-lamp.bak` | 67 | 09-17 16:44 | 光敏阈值控灯：电位器设阈值、光敏实测比较、LED1 联动，OLED 显 Set/Real/Lamp |
| `main.c4.dac-adc-loop.bak` | 85 | 09-17 17:07 | DAC→ADC 回环精度：设定电压循环 +0.1V，PA6 采回，显 SET/RD/ERR |
| `main.c5.low-power.bak` | 57 | 09-18 11:44 | 低功耗：PWR + EXTI，flag 2/3/4 → SLEEP / STOP / STANDBY |
| `main.c6.adc-dma.bak` | 41 | 09-18 16:05 | ADC1 + DMA 采样，串口打印 ADC1ConvertedValue |

## 已清掉的模块备份（9-20）

| 清掉的文件 | 想找回来怎么办 |
|---|---|
| `esp8266.c.*`（9 个）/ `esp8266.h.*`（7 个） | `git log --oneline -- stm32/f1x-f4x/f407/HARDWARE/esp8266.c` 找到对应时间的提交，再 `git show <commit>:<路径>` |
| `web.c.html版.bak` / `web.h.html版.bak`（板子自己发网页那版） | `git show 2eb8d09~1:stm32/f1x-f4x/f407/WEB/web.c` |
| `OLED.c.bak`（9-11 老驱动，迁移前） | `git log --oneline -- stm32/f1x-f4x/f407/HARDWARE/OLED.c` |
| 上面这 19 个文件本体 | 在**废纸篓**里：`~/.Trash/back_bak_20260920/`（拖回来即可还原） |

## 怎么从 git 取回任意旧版本

```bash
cd /Volumes/Keil_v5/projects/cstudy
git log --oneline -- stm32/f1x-f4x/f407/HARDWARE/esp8266.c        # 看历史
git show <commit>:stm32/f1x-f4x/f407/HARDWARE/esp8266.c > /tmp/x.c # 导出某一版
```

`main.c1`~`main.c6` 也能取（它们是 2026-09-20 才从仓库移除的）：

```bash
git show 39eb173~1:stm32/f1x-f4x/f407/USER/main.c1 > /tmp/main.c1
```

## 编码提醒

本目录多数是 **GBK**（和工程一致）；`main.c1.f103-link.bak`、`main.c.timer.bak`、`main.c.etim.bak` 是 **UTF-8**。
拿回去当 `USER/main.c` 之前先转成 GBK，否则 Keil 里中文注释全是乱码：

```bash
iconv -f UTF-8 -t GBK main.c1.f103-link.bak > ../main.c
```
