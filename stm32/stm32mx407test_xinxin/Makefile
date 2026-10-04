# ============================================================
#  STM32F407 工程 Makefile（包装 CMake + Ninja + 烧录 + 串口）
#
#  为什么这么写（都是踩过的坑）：
#   1) 必须走 `cmake --preset Debug`：工具链写在 CMakePresets.json 的
#      "toolchainFile" 里。手工 `cmake -S . -B build/Debug -G Ninja` 少写
#      -DCMAKE_TOOLCHAIN_FILE 时，CMake 会拿宿主编译器（AppleClang）配置，
#      而且 configure 还会"成功"，到 build 才炸：
#         startup_stm32f407xx.s:27:3: error: unknown directive
#   2) preset 每次都会重新 configure，顺带解决 `file(GLOB BSP/Src/*.c)` 的坑：
#      新增源文件后不重新 configure，它不会被编进工程（改了等于没改）。
#   3) 工具链不在默认 PATH 里（homebrew 那个缺 newlib），这里自己加上。
# ============================================================

export PATH := $(HOME)/dev/toolchains/bin:$(PATH)

# ---------- 配置 ----------
BUILD_TYPE ?= Debug          # Debug | Release（Release 用 -O3，体积/速度都更好）
BUILD_DIR ?= build/$(BUILD_TYPE)
PROJECT   ?= stm32mx407test_xinxin
ELF       := $(BUILD_DIR)/$(PROJECT).elf
HEX       := $(BUILD_DIR)/$(PROJECT).hex
BIN       := $(BUILD_DIR)/$(PROJECT).bin

# 烧录工具：openocd（当前接的是 ST-Link）| st-flash | STM32_Programmer_CLI | pyocd
FLASH_TOOL     ?= openocd
OPENOCD_IF     ?= interface/stlink.cfg   # 换 J-Link 改成 interface/jlink.cfg（并加 -c "transport select swd"）
OPENOCD_TARGET ?= target/stm32f4x.cfg

# printf 走 UART5 = PC12(TX)/PD2(RX)；CH340 USB-TTL 就是下面这个口
SERIAL_PORT ?= /dev/cu.wchusbserial1110
SERIAL_BAUD ?= 115200

.PHONY: all configure build release hex bin size clean distclean rebuild flash reset openocd serial help

all: build

# ---------- 编译 ----------
configure:
	@cmake --preset $(BUILD_TYPE)

build:
	@echo ">>> configure + build（cmake --preset $(BUILD_TYPE)）"
	@cmake --preset $(BUILD_TYPE)
	@cmake --build --preset $(BUILD_TYPE)

# Release 构建（-O3）；烧录同样可以 make flash BUILD_TYPE=Release
release:
	@$(MAKE) build BUILD_TYPE=Release

hex: build
	@arm-none-eabi-objcopy -O ihex $(ELF) $(HEX)
	@echo ">>> $(HEX)"

bin: build
	@arm-none-eabi-objcopy -O binary $(ELF) $(BIN)
	@echo ">>> $(BIN)"

size: build
	@arm-none-eabi-size $(ELF)

clean:
	@rm -rf $(BUILD_DIR)

distclean:
	@rm -rf build

rebuild: clean build

# ---------- 烧录 ----------
# flash 依赖 bin/hex：原来只依赖 build，recipe 里却用 $(BIN)/$(HEX)，
# 轻则报文件不存在，重则把上一次留下的旧 .bin 烧进去。
flash: bin hex
	@echo ">>> Flashing via $(FLASH_TOOL) ..."
ifeq ($(FLASH_TOOL),openocd)
	openocd -f $(OPENOCD_IF) -f $(OPENOCD_TARGET) \
		-c "program $(ELF) verify reset exit"
else ifeq ($(FLASH_TOOL),st-flash)
	st-flash --reset write $(BIN) 0x08000000
else ifeq ($(FLASH_TOOL),STM32_Programmer_CLI)
	STM32_Programmer_CLI -c port=SWD -w $(HEX) -v -rst
else ifeq ($(FLASH_TOOL),pyocd)
	pyocd flash -t stm32f407zg $(BIN) 0x08000000
else
	@echo "Unknown FLASH_TOOL: $(FLASH_TOOL)" && exit 1
endif

reset:
	openocd -f $(OPENOCD_IF) -f $(OPENOCD_TARGET) -c "init" -c "reset run" -c "shutdown"

openocd:
	openocd -f $(OPENOCD_IF) -f $(OPENOCD_TARGET)

# ---------- 串口监视（看 printf）----------
serial:
	@echo ">>> $(SERIAL_PORT) @ $(SERIAL_BAUD)   退出：先按 Ctrl-A 再按 \\"
	@screen $(SERIAL_PORT) $(SERIAL_BAUD)

# ---------- 帮助 ----------
help:
	@echo "Available targets:"
	@echo "  make            - 配置 + 编译（PRESET=$(BUILD_TYPE)）"
	@echo "  make build      - 同上"
	@echo "  make release    - Release(-O3) 构建（= make build BUILD_TYPE=Release）"
	@echo "  make flash      - 编译 + 生成 bin/hex + 烧录（$(FLASH_TOOL)）"
	@echo "  make reset      - 复位并运行"
	@echo "  make serial     - 打开串口 $(SERIAL_PORT) @ $(SERIAL_BAUD) 看 printf"
	@echo "  make size       - 看 Flash/RAM 占用"
	@echo "  make hex / bin  - 只生成 hex / bin"
	@echo "  make clean      - 删 $(BUILD_DIR)"
	@echo "  make distclean  - 删整个 build/"
	@echo "  make openocd    - 只起 openocd 服务（给 GDB / VS Code 用）"
	@echo ""
	@echo "可覆盖变量："
	@echo "  FLASH_TOOL=openocd|st-flash|STM32_Programmer_CLI|pyocd"
	@echo "  OPENOCD_IF=interface/stlink.cfg | interface/jlink.cfg"
	@echo "  SERIAL_PORT=/dev/cu.wchusbserial1110"
