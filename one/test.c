#include <stdio.h>

int main() {
    // 打印 long 类型在当前环境下占用的字节数
    printf("long 类型占用 %d 个字节\n", (int)sizeof(long long));
    return 0;
}
