#include <stdio.h>

int main(void)
{
  printf("测试下输出输出:");
  int a, b;
  // scanf 需要地址，所以加 &
  scanf("%d%d", &a, &b);

  // printf 只需要值，所以去掉 &
  printf("%d,%d\n", a, b);

  char c;
  printf("请输入一个字符：");
  // scanf 需要地址加 &，%c 前加空格防止读取到残留的换行符
  getchar(); // 读取掉之前输入的换行符
  scanf("%c", &c);

  // printf 只需要值，去掉 &
  printf("测试下字符%c\n", c);

  return 0;
}