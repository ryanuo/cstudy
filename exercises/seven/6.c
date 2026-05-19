#include <stdio.h>

int f(int a)
{
  int b = 0;
  static int c = 3;
  b = b + 1;
  c = c + 1;
  printf("%d\n", c);
  return (a + b + c);
}

int main()
{
  f(1);
  return 0;
}