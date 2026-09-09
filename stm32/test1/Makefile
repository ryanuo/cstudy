# STM32 Test1 Quick Build

BUILD_DIR   := build/Debug
TARGET     := test1.elf

OPENOCD    := openocd
OCD_IF     := interface/stlink.cfg
OCD_TARGET := target/stm32f1x.cfg

JOBS       := $(shell sysctl -n hw.ncpu)

.PHONY: all build flash clean

all: flash

# 只在 build 目录不存在时配置
$(BUILD_DIR)/CMakeCache.txt:
	cmake --preset Debug

build: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR) -j$(JOBS)

flash: build
	$(OPENOCD) -f $(OCD_IF) -f $(OCD_TARGET) -c "program $(BUILD_DIR)/$(TARGET) verify reset exit"

clean:
	rm -rf build
