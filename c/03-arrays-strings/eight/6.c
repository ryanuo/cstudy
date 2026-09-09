#include <stdio.h>

long long climbStairs(int n) {
    if (n <= 2) {
        return n;
    }

    // 1. 将内部计算变量也升级为 long long，防止计算中途溢出
    long long prev1 = 2;
    long long prev2 = 1;
    long long current = 0;

    for (int i = 3; i <= n; i++) {
        current = prev1 + prev2;
        prev2 = prev1;
        prev1 = current;
    }

    return current;
}

int main() {
    int n;
    printf("请输入楼梯阶数 n: ");
    scanf("%d", &n);

    // 2. 接收返回值的变量类型改为 long long
    long long result = climbStairs(n);
    
    // 3. 使用正确的 %lld 格式打印长整型
    printf("爬到第 %d 阶共有 %lld 种不同的方法。\n", n, result);

    return 0;
}