#include <stdio.h>

int main()
{
    printf("char 占用字节数: %zu\n", sizeof(char));
    printf("short 占用字节数: %zu\n", sizeof(short));
    printf("int 占用字节数: %zu\n", sizeof(int));
    printf("long 占用字节数: %zu\n", sizeof(long));
    printf("long long 占用字节数: %zu\n", sizeof(long long));
    printf("float 占用字节数: %zu\n", sizeof(float));
    printf("double 占用字节数: %zu\n", sizeof(double));
    printf("long double 占用字节数: %zu\n", sizeof(long double));
    return 0;
}

// char 占用字节数: 1
// short 占用字节数: 2
// int 占用字节数: 4
// long 占用字节数: 8
// long long 占用字节数: 8
// float 占用字节数: 4
// double 占用字节数: 8
// long double 占用字节数: 16