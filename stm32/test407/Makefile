######################################################################
# STM32F407ZET6 Makefile + OpenOCD 烧录
######################################################################

TARGET   = test407
BUILD    = build

# 工具链
PREFIX   = arm-none-eabi-
CC       = $(PREFIX)gcc
AS       = $(PREFIX)gcc -x assembler-with-cpp
CP       = $(PREFIX)objcopy
SZ       = $(PREFIX)size
HEX      = $(CP) -O ihex
BIN      = $(CP) -O binary -S

# 使用本地工具链（homebrew 版缺少 newlib 头文件）
TOOLCHAIN = $(HOME)/dev/toolchains/bin/
CC    = $(TOOLCHAIN)$(PREFIX)gcc
AS    = $(TOOLCHAIN)$(PREFIX)gcc -x assembler-with-cpp
CP    = $(TOOLCHAIN)$(PREFIX)objcopy
SZ    = $(TOOLCHAIN)$(PREFIX)size

# MCU 定义 (F407 Cortex-M4F)
MCU      = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# 链接脚本
LDSCRIPT = STM32F407xx_FLASH.ld

# 源文件(自动扫描各目录)
SRCS_C   = \
  $(wildcard Core/Src/*.c) \
  $(wildcard Drivers/STM32F4xx_HAL_Driver/Src/*.c) \
  $(wildcard models/*.c) \
  $(wildcard App/*.c)

# 启动文件
SRCS_S   = $(wildcard startup_*.s)

SRCS     = $(SRCS_C) $(SRCS_S)

# 搜索路径(自动提取所有包含 .c 的目录)
VPATH    = $(sort $(dir $(SRCS_C)))

# 头文件路径
INCS     = \
  -ICore/Inc \
  -IApp \
  -Imodels \
  -IDrivers/STM32F4xx_HAL_Driver/Inc \
  -IDrivers/CMSIS/Device/ST/STM32F4xx/Include \
  -IDrivers/CMSIS/Include

# 编译标志
DEFS     = -DUSE_HAL_DRIVER -DSTM32F407xx
CFLAGS   = $(MCU) $(DEFS) $(INCS) -Wall -fdata-sections -ffunction-sections -Og
LDFLAGS  = $(MCU) -specs=nano.specs -u _printf_float -T$(LDSCRIPT) -lc -lm -lnosys \
           -Wl,-Map=$(BUILD)/$(TARGET).map,--cref -Wl,--gc-sections

# 对象文件
OBJS = $(addprefix $(BUILD)/,$(notdir $(SRCS:.c=.o)))
OBJS := $(OBJS:.s=.o)

# 默认目标
all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).hex $(BUILD)/$(TARGET).bin

# 编译 .c
$(BUILD)/%.o: %.c Makefile | $(BUILD)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD)/$(notdir $(<:.c=.lst)) $< -o $@

# 汇编 .s
$(BUILD)/%.o: %.s Makefile | $(BUILD)
	$(AS) -c $(CFLAGS) $< -o $@

# 链接
$(BUILD)/$(TARGET).elf: $(OBJS) Makefile
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SZ) $@

# hex / bin
$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf | $(BUILD)
	$(HEX) $< $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf | $(BUILD)
	$(BIN) $< $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

######################################################################
# OpenOCD 烧录 (ST-Link V2)
######################################################################
OPENOCD       = openocd
OPENOCD_IF    = interface/stlink.cfg
OPENOCD_TARGET = target/stm32f4x.cfg

# 一键编译 + 烧录（烧完自动运行）
flash: all
	$(OPENOCD) -f $(OPENOCD_IF) -f $(OPENOCD_TARGET) \
		-c "program $(BUILD)/$(TARGET).elf verify" \
		-c "reset run" \
		-c "sleep 200" \
		-c "shutdown"

# 仅烧录（不重新编译）
burn: all
	$(OPENOCD) -f $(OPENOCD_IF) -f $(OPENOCD_TARGET) \
		-c "program $(BUILD)/$(TARGET).elf verify" \
		-c "reset run" \
		-c "sleep 200" \
		-c "shutdown"

# 调试模式 (GDB 端口 3333)
debug:
	$(OPENOCD) -f $(OPENOCD_IF) -f $(OPENOCD_TARGET)

# 别名
run: flash

.PHONY: all clean flash burn debug run
