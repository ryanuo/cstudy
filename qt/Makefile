# ============================================================
# Qt 练习项目 Makefile
#
# 目录:
#   NN_name/
#       main.cpp
#
# 例如:
#   01_hello/
#   02_login/
#   03_network/
#
# CMake:
#   01_hello -> hello
#   02_login -> login
#
# 用法:
#   make build
#   make build T=01
#   make build T=hello
#
#   make run T=01
#   make run T=hello
#
#   make package T=01
#   make package T=hello
#
#   make list
#   make clean
#   make rebuild
# ============================================================

QT_DIR := $(HOME)/Qt/6.11.1/macos

BUILD_DIR := build
LIB_DIR := lib
PACKAGE_DIR := package

CMAKE := cmake

.PHONY: build run package list clean rebuild

# ============================================================
# 默认目标
# ============================================================

build:
	@if [ -n "$(T)" ]; then \
		echo "======================================"; \
		echo "编译练习: $(T)"; \
		echo "======================================"; \
		TARGET=""; \
		for d in [0-9][0-9]_*/; do \
			[ -d "$$d" ] || continue; \
			dir="$${d%/}"; \
			name="$${dir#*_}"; \
			number="$${dir%%_*}"; \
			if [ "$(T)" = "$$dir" ] || \
			   [ "$(T)" = "$$name" ] || \
			   [ "$(T)" = "$$number" ]; then \
				TARGET="$$name"; \
				break; \
			fi; \
		done; \
		if [ -z "$$TARGET" ]; then \
			echo "错误: 未找到练习 T=$(T)"; \
			echo ""; \
			$(MAKE) list; \
			exit 1; \
		fi; \
		echo "CMake Target: $$TARGET"; \
		$(CMAKE) -S . -B $(BUILD_DIR) \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_PREFIX_PATH="$(QT_DIR)"; \
		$(CMAKE) --build $(BUILD_DIR) \
			--target "$$TARGET" \
			-j$$(sysctl -n hw.ncpu); \
	else \
		echo "======================================"; \
		echo "编译全部练习"; \
		echo "======================================"; \
		$(CMAKE) -S . -B $(BUILD_DIR) \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_PREFIX_PATH="$(QT_DIR)"; \
		$(CMAKE) --build $(BUILD_DIR) \
			-j$$(sysctl -n hw.ncpu); \
	fi


# ============================================================
# 运行
#
# make run T=01
# make run T=hello
# ============================================================

run:
	@if [ -z "$(T)" ]; then \
		echo "用法: make run T=01"; \
		exit 1; \
	fi; \
	TARGET=""; \
	DIR=""; \
	for d in [0-9][0-9]_*/; do \
		[ -d "$$d" ] || continue; \
		dir="$${d%/}"; \
		name="$${dir#*_}"; \
		number="$${dir%%_*}"; \
		if [ "$(T)" = "$$dir" ] || \
		   [ "$(T)" = "$$name" ] || \
		   [ "$(T)" = "$$number" ]; then \
			TARGET="$$name"; \
			DIR="$$dir"; \
			break; \
		fi; \
	done; \
	if [ -z "$$TARGET" ]; then \
		echo "错误: 未找到练习 T=$(T)"; \
		$(MAKE) list; \
		exit 1; \
	fi; \
	$(MAKE) build T=$(T); \
	APP="$(LIB_DIR)/$$DIR/$$TARGET.app"; \
	BIN="$(LIB_DIR)/$$DIR/$$TARGET"; \
	if [ -d "$$APP" ]; then \
		echo "运行 $$APP"; \
		open "$$APP"; \
	elif [ -x "$$BIN" ]; then \
		echo "运行 $$BIN"; \
		"$$BIN"; \
	else \
		echo "错误: 找不到可执行文件"; \
		exit 1; \
	fi


# ============================================================
# 打包
#
# make package T=01
# make package T=hello
# ============================================================

package:
	@if [ -z "$(T)" ]; then \
		echo "用法: make package T=01"; \
		exit 1; \
	fi; \
	echo "======================================"; \
	echo "打包练习: $(T)"; \
	echo "======================================"; \
	./package.sh "$(T)"


# ============================================================
# 列出练习
# ============================================================

list:
	@echo "======================================"
	@echo "已发现的练习"
	@echo "======================================"
	@for d in [0-9][0-9]_*/; do \
		[ -d "$$d" ] || continue; \
		if ls "$$d"*.cpp >/dev/null 2>&1; then \
			dir="$${d%/}"; \
			number="$${dir%%_*}"; \
			name="$${dir#*_}"; \
			echo "  $$number  ->  $$name  ($$dir)"; \
		fi; \
	done


# ============================================================
# 清理
# ============================================================

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(LIB_DIR)
	rm -rf $(PACKAGE_DIR)


# ============================================================
# 全量重新构建
# ============================================================

rebuild: clean
	$(MAKE) build