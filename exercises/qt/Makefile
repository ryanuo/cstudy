# Qt 练习项目 Makefile 快捷命令（动态发现练习，新增练习无需改配置）
#
# 目录约定:
#   NN_name/  每个练习一个子目录, NN 为数字编号, 内含 .cpp 源文件
#   lib/      编译产物, 由 cmake 自动输出到 lib/NN_name/
#
# 用法:
#   make build        配置并编译全部练习
#   make run T=01     运行编号为 01 的练习 (也可用部分名, 如 T=hello)
#   make list         列出所有已发现的练习
#   make clean        删除 build 目录和 lib 产物
#   make rebuild      清理后重新配置编译

# Qt 安装路径(Apple Silicon macOS)
QT_DIR := $(HOME)/Qt/6.11.1/macos

BUILD_DIR := build

.PHONY: build run list clean rebuild

# 默认目标
build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_PREFIX_PATH="$(QT_DIR)"
	cmake --build $(BUILD_DIR)

# 列出所有练习子目录（NN_ 开头 且 含 .cpp 源文件）
list:
	@echo "已发现的练习:"; \
	for d in [0-9][0-9]_*/; do \
		[ -n "$$(ls "$$d"*.cpp 2>/dev/null)" ] && echo "  $${d%/}"; \
	done; \
	true

# 运行指定练习: make run T=01 或 make run T=hello
# 按编号前缀(如 01)或目录名(如 hello)匹配, 自动找到对应可执行文件
T ?=
run: build
	@BIN=""; \
	for d in [0-9][0-9]_*/; do \
		[ -n "$$(ls "$$d"*.cpp 2>/dev/null)" ] || continue; \
		dir="$${d%/}"; \
		if [ "$(T)" = "$$dir" ] || [ "$${dir#*_}" = "$(T)" ] \
		   || [ "$${dir%%_*}" = "$(T)" ]; then \
			BIN="lib/$$dir/$${dir#*_}"; break; \
		fi; \
	done; \
	if [ -z "$$BIN" ] || [ ! -x "$$BIN" ]; then \
		echo "未找到练习: T=$(T)"; echo "可用:"; \
		for d in [0-9][0-9]_*/; do \
			[ -n "$$(ls "$$d"*.cpp 2>/dev/null)" ] && \
				echo "  T=$${d%%_*} ($${d%/})"; \
		done; \
		exit 1; \
	fi; \
	echo "运行 $$BIN"; "$$BIN"

# 清理 build 和 lib 产物
clean:
	rm -rf $(BUILD_DIR)
	rm -rf lib

# 全量重建
rebuild: clean
	cmake -S . -B $(BUILD_DIR) -DCMAKE_PREFIX_PATH="$(QT_DIR)"
	cmake --build $(BUILD_DIR)
