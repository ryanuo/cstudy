# STM32 Test1 Quick Build

BUILD_DIR   := build/Debug
BUILD_TYPE  := Debug
TARGET     := test1.elf

OPENOCD    := openocd
OCD_IF     := interface/stlink.cfg
OCD_TARGET := target/stm32f1x.cfg

.PHONY: all config build flash clean

all: config build

config:
	cmake --preset $(BUILD_TYPE)

build:
	cmake --build $(BUILD_DIR) -j$(shell sysctl -n hw.ncpu)

flash: build
	$(OPENOCD) -f $(OCD_IF) -f $(OCD_TARGET) -c "program $(BUILD_DIR)/$(TARGET) verify reset exit"

clean:
	rm -rf build
