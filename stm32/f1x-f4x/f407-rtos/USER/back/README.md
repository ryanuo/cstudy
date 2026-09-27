# USER/back —— 旧版本留档

这里放**改动前**的旧文件。项目 `.gitignore` 里写了 `*.bak`，所以这些文件**只存在本机、不进仓库**。

- 想用某一版：`cp main.c.esp-fixed.bak ../main.c`，然后回 Keil 重新 Rebuild + Download
- 清历史：2026-09-20 删过两批（模块类备份 19 个；WiFi 迭代版 8 个），都只是**挪进废纸篓**，见文末

## 一、ESP8266 WiFi 那轮（9-19，只留 4 个关键节点）

| 文件 | 行数 | 时间 | 是什么 |
|---|---|---|---|
| `main.c.esp.bak` | 48 | 09-19 15:29 | 第一次接 ESP8266：只会 AT 收发，用自写假延时、失败就死等 |
| `main.c.diag1.bak` | 157 | 09-19 15:41 | 诊断固件 v2：OLED 显示收到的原始字节（hex）+ 长度，用来分清「模块不说话」还是「我们收不到」 |
| `main.c.diag3.bak` | 189 | 09-19 15:58 | 诊断固件 v3：加 SELF（片内回环自测）/ PIN（读引脚电平）/ BOOT（74880 抓模块启动信息） |
| `main.c.esp-fixed.bak` | 171 | 09-19 15:37 | 修好延时与重试后的正式 WiFi 固件（AT → CWMODE → CWJAP → CIFSR） |

> 中间的 `main.c.wifi-v1~v6` / `wifi-web` / `wifi-web2`（连 WiFi 的 6 次迭代、加手机网页那一版）
> 已于 2026-09-20 清掉；它们对应的是每次改动前的 `USER/main.c`，需要时用
> `git log --oneline -- stm32/f1x-f4x/f407/USER/main.c` 找当时的提交即可。

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

## 已清掉的文件（2026-09-20）

| 清掉的文件 | 想找回来怎么办 |
|---|---|
| `esp8266.c.*`（9）/ `esp8266.h.*`（7） | `git log --oneline -- stm32/f1x-f4x/f407/HARDWARE/esp8266.c` 找对应时间的提交，再 `git show <commit>:<路径>` |
| `web.c.html版.bak` / `web.h.html版.bak`（板子自己发网页那版） | `git show 2eb8d09~1:stm32/f1x-f4x/f407/WEB/web.c` |
| `OLED.c.bak`（9-11 老驱动，迁移前） | `git log --oneline -- stm32/f1x-f4x/f407/HARDWARE/OLED.c` |
| `main.c.wifi-v1~v6/wifi-web/wifi-web2.bak`（8 个） | 同上，`git log` 查 `USER/main.c` 的历史 |
| 上面这些文件本体 | 在**废纸篓**里：`~/.Trash/back_bak_20260920/`（拖回来即可还原） |

## 怎么从 git 取回任意旧版本

```bash
cd /Volumes/Keil_v5/projects/cstudy
git log --oneline -- stm32/f1x-f4x/f407/HARDWARE/esp8266.c        # 看历史
git show <commit>:stm32/f1x-f4x/f407/HARDWARE/esp8266.c > /tmp/x.c # 导出某一版
```

`main.c1`~`main.c6` 也能取（2026-09-20 才从仓库移除）：

```bash
git show 39eb173~1:stm32/f1x-f4x/f407/USER/main.c1 > /tmp/main.c1
```

## 编码提醒

本目录多数是 **GBK**（和工程一致）；`main.c1.f103-link.bak`、`main.c.timer.bak`、`main.c.etim.bak` 是 **UTF-8**。
拿回去当 `USER/main.c` 之前先转成 GBK，否则 Keil 里中文注释全是乱码：

```bash
iconv -f UTF-8 -t GBK main.c1.f103-link.bak > ../main.c
```
