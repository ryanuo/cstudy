## 计划：分模块 + 模式切换

### 新增文件（`src/func/`）

| 文件 | 函数 | 功能 |
|------|------|------|
| `src/func/oled.h/c` | `OLED_Init()`, `OLED_ShowMode(int mode)` | OLED 显示 |
| `src/func/fan.h/c` | `Fan_Init()`, `Fan_Forward()`, `Fan_Reverse()`, `Fan_Stop()` | 风扇 PB13/PB12 |
| `src/func/buzzer.h/c` | `Buzzer_Init()`, `Buzzer_On()`, `Buzzer_Off()` | 蜂鸣器 PA1 |
| `src/func/led.h/c` | `LED_Init()`, `LED_Toggle()`, `LED_On()`, `LED_Off()` | PA10 LED + PC13 贴片灯 |

### 模式定义（1-6）

| 模式 | 外设行为 |
|------|----------|
| 1 | 风扇正转 |
| 2 | 风扇反转 |
| 3 | 风扇停转 |
| 4 | 蜂鸣器响 |
| 5 | LED 翻转（PA10 + PC13 一起闪） |
| 6 | OLED 显示当前模式号 |

### main.c while 循环

```c
#include "func/oled.h"
#include "func/fan.h"
#include "func/buzzer.h"
#include "func/led.h"

int mode = 1;
while (1) {
    switch (mode) {
        case 1: Fan_Forward();  break;
        case 2: Fan_Reverse();  break;
        case 3: Fan_Stop();      break;
        case 4: Buzzer_On();     break;
        case 5: LED_Toggle();    break;
        case 6: /* 仅刷新 OLED */ break;
    }
    OLED_ShowMode(mode);
    HAL_Delay(1000);
    mode = (mode % 6) + 1;  // 1→2→3→4→5→6→1...
}
```

### 修改文件

- `cmake/stm32cubemx/CMakeLists.txt` — 添加 `src/func/*.c` 到编译
- `Core/Src/main.c` — 清空当前 while 循环，改为 switch
- 所有外设独立文件，单独调整互不影响

确认后开写。
