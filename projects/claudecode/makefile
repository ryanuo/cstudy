CC=gcc

CFLAGS=-Wall -Wextra -g

INCLUDE=-Iinclude

SRC=src/main.c \
	src/agent.c \
	src/llm.c \
	src/tool.c \
	src/file.c \
	src/shell.c

TARGET=claude

all:
	$(CC) $(CFLAGS) $(INCLUDE) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)