#include <stdio.h>

int main()
{
  char a[20], b[20], c[20];

  printf("请输入三个单词，用空格隔开: ");

  if (scanf("%s %s %s", a, b, c) == 3)
  {
    printf("倒序输出: %s %s %s\n", c, b, a);
  }
  else
  {
    printf("输入格式错误！\n");
  }

  return 0;
}
