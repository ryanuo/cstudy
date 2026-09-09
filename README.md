# C 习题知识点总结

本 README 汇总当前工作区中的 C / C++ / Qt / STM32 / Linux 习题，按知识方向归类整理，方便后续复习与查漏补缺。

> **目录命名约定**：所有目录保留英文名（数字前缀 + 短横线 + 小写关键词）；本 README 在每个模块下用中文描述内容。
> **构建产物**：`build/`、`*.o`、`*.dSYM` 等已被 `.gitignore` 排除，无需手动清理。
> **辅助目录**：`.workbuddy/` 存放项目本地 memory，已 `.gitignore`。

# 目录总览

```
cstudy/
├── c/                              C 语言基础（按 topic 拆分）
│   ├── 01-io-format/               基础 I/O + 数据类型
│   ├── 02-control-flow/            分支 / 循环 / switch
│   ├── 03-arrays-strings/          数组 + 字符串
│   ├── 04-math/                    数学几何 + 数字位运算
│   ├── 05-pointers/                指针专题
│   ├── 06-structs-unions/          结构体 + 联合
│   ├── 07-file-io/                 文件 I/O + 权限
│   ├── 08-linked-list/             链表（含双向 / 循环）
│   ├── 09-tree/                    二叉查找树
│   └── 10-commands/                自实现命令
├── cpp/                            C++ 基础
├── linux/                          Linux 系统编程（含 024 章）
├── qt/                             Qt 桌面应用
├── stm32/                          STM32 嵌入式开发
└── projects/                       独立项目
    ├── claudecode/                 C 版 Claude Code
    └── studyTask/                  时间工具
```

每个 `c/<topic>/` 下仍按原章节分子目录（如 `c/04-math/one/`、`c/04-math/four/`），方便追溯源码来自哪一章。

# 各目录核心内容

## C 语言基础（按 topic）

| Topic | 来源章节 | 主要内容 |
| --- | --- | --- |
| 01-io-format 基础 I/O + 数据类型 | two、three | IEEE-754 存储注释、时间换算、printf/scanf 综合 |
| 02-control-flow 流程控制 | five | `for` / `while` + `continue` / `break` 综合 |
| 03-arrays-strings 数组与字符串 | six、eight、nine | 数组搜索、字符数组、数组反转 |
| 04-math 数学与数字 | one、four、seven、eighteen | 几何计算（面积/半径/BMI/球体体积）、数字位平方求和 |
| 05-pointers 指针专题 | ten | 函数返回指针、指针数组、指针运算 |
| 06-structs-unions 结构体与联合 | eleven | `struct Product`、综合数据处理 |
| 07-file-io 文件 I/O 与权限 | twelve、thirteen、nineteen | FILE 复制、登录注册、`access` / `open` 权限 |
| 08-linked-list 链表 | fourteen、fifteen、sixteen | 单/双/通用链表、公交线路管理、双向循环约瑟夫环 |
| 09-tree 二叉树 | seventeen | 二叉查找树插入 + 先/中/后序遍历 |
| 10-commands 命令实现 | twenty | 自实现 `ls` / `rename` / `rm`，时间工具 |

## C++ 基础

| 模块 | 包含目录 | 主要内容 |
| --- | --- | --- |
| C++ 入门 | cpp/01 – cpp/05 | 类与构造函数、类设计、函数模板、STL 容器、智能指针 |

## Linux 系统编程

| 模块 | 包含目录 | 主要内容 |
| --- | --- | --- |
| 杂项 IPC + 宏 | linux/024 | `pipe` + `fork` 父子进程通信、`max` / `swap` 宏练习 |
| System V IPC | linux/3 | 共享内存、信号量、消息队列 |
| 管道与多进程 | linux/4 – linux/9 | 匿名管道、命名管道、`fork`、`signal`、文件描述符 |
| 套接字与线程池 | linux/10 – linux/12 | TCP 客户端/服务器、线程池、综合小例 |
| 多进程与线程综合 | linux/twenty-one | `fork`、进程池、线程池综合 |
| POSIX 与高级 IPC | linux/twenty-two | 命名管道读写、POSIX 信号量、文件复制 |

## Qt 桌面应用

| 模块 | 包含目录 | 主要内容 |
| --- | --- | --- |
| Qt 入门小项目 | qt/01_hello、qt/02_counter、qt/03_jisuanqi、qt/04_login | Hello World、计数器、计算器、登录对话框 |
| Qt 综合项目 | qt/new-pro | 数据库登录/注册示例（CMake + 多对话框 + 资源） |

## STM32 嵌入式开发

| 模块 | 包含目录 | 主要内容 |
| --- | --- | --- |
| STM32F103 基础工程 | stm32/test1 | HAL 库模板（GPIO / UART / OLED / 按键 / 风扇等） |
| STM32F407 综合工程 | stm32/test407 | HAL 库综合（多外设 + App 分层） |

## 独立项目

| 模块 | 包含目录 | 主要内容 |
| --- | --- | --- |
| Claude Code C Edition | projects/claudecode | `agent` / `context` / `llm` / `tool` / `file` / `shell` 模块化 CLI Agent |
| 时间戳与字符串互转 | projects/studyTask | 时间戳转换、字符串与时间互转、工具函数 |
