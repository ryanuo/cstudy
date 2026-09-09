# ==============================
# C++ 单文件编译 Makefile
#
# 使用:
# make 目录/文件.cpp
#
# 示例:
# make 1/1.cpp
#
# 输出:
# lib/1/1
# ==============================


CXX := clang++

CXXFLAGS := -std=c++14 -Wall -Wextra -g


OUT_DIR := lib


# 获取输入文件
SRC := $(firstword $(MAKECMDGOALS))


# 去掉 .cpp
TARGET_NAME := $(basename $(SRC))


# 输出路径
TARGET := $(OUT_DIR)/$(TARGET_NAME)



.PHONY: $(MAKECMDGOALS)
.PHONY: clean



$(MAKECMDGOALS):

	@echo "=============================="
	@echo " Build: $(SRC)"
	@echo " Output: $(TARGET)"
	@echo "=============================="

	# 创建目标目录
	@mkdir -p $(dir $(TARGET))

	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

	@echo "Run:"
	@echo "./$(TARGET)"
	@echo "=============================="
	@./$(TARGET)



clean:

	rm -rf $(OUT_DIR)

	@echo "clean success"