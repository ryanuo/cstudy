#include <stdio.h>

// 方法一：指针强转法
int check_endian_pointer() {
    int i = 1;
    // 取出 i 的地址，强转为 char* 指针，解引用只取第一个字节
    return (*(char*)&i); 
}

// 方法二：联合体（union）法
int check_endian_union() {
    union {
        int i;
        char c;
    } un;
    un.i = 1;
    // 联合体成员共享内存，读取 char c 相当于读取 int i 的第一个字节
    return un.c;
}

int main() {
    if (check_endian_pointer() == 1) {
        printf("当前系统是：小端模式 (Little-Endian)\n");
    } else {
        printf("当前系统是：大端模式 (Big-Endian)\n");
    }
    
    // 验证第二种方法
    if (check_endian_union() == 1) {
        printf("验证结果：小端模式\n");
    } else {
        printf("验证结果：大端模式\n");
    }

    return 0;
}