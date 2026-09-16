# 快捷命令：make build / run / clean / fresh / info

QT_PREFIX ?= $(shell brew --prefix qt 2>/dev/null)
ifeq ($(QT_PREFIX),)
  QT_PREFIX := $(HOME)/Qt/6.11.1/macos
endif

# 强制只用 Homebrew Qt（避免 HOME Qt 干扰 SerialPort 查找）
CMAKE_PREFIX = $(QT_PREFIX)
# 构建目录默认放本机磁盘：
# 本工程位于 SMB 网络盘（/Volumes/Keil_v5），CMake 在 smbfs 上写出的中间文件会丢
# （实测 build/.qt/appqtlink_qml.cmake、CMakeFiles/Makefile2 编译时找不到），
# 所以默认把构建目录放到本机。要在盘内构建：make build BUILD_DIR=build
LOCAL_BUILD ?= $(HOME)/dev/qt/qt-link-build
BUILD_DIR   ?= $(LOCAL_BUILD)
APP         ?= appqtlink
BIN          = $(BUILD_DIR)/bin/$(APP)
JOBS        ?= $(shell sysctl -n hw.ncpu 2>/dev/null || echo 4)
# 含 QtShadcn 模块目录的父路径（third_party 布局）
QML_IMPORT  ?= $(CURDIR)/$(BUILD_DIR)/third_party/qtshadcn/src

.PHONY: all build run clean fresh info configure

all: build

# 仅当构建缓存不存在时才完整生成 CMake 配置
configure:
	@if [ ! -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		cmake -S . -B $(BUILD_DIR) \
			-DCMAKE_PREFIX_PATH="$(CMAKE_PREFIX)" \
			-DCMAKE_EXPORT_COMPILE_COMMANDS=ON; \
	fi

# 增量编译：代码未变动时直接跳过
build: configure
	cmake --build $(BUILD_DIR) -j$(JOBS)

run: build
	QML_IMPORT_PATH="$(QML_IMPORT)" $(BIN)

clean:
	rm -rf $(BUILD_DIR)

fresh: clean build

info:
	@echo "Qt:         $(QT_PREFIX)"
	@echo "Build Dir:  $(BUILD_DIR)"
	@echo "Binary:     $(BIN)"
	@echo "QML Import: $(QML_IMPORT)"
	@echo "Jobs:       $(JOBS)"
