#include "main.h"
#include <stdio.h>

extern UART_HandleTypeDef huart5; // CubeMX 生成的 UART5 句柄

/* ============================================================
 *   printf 重定向到 UART5
 *
 *   本工程用 arm-none-eabi-gcc + newlib-nano（--specs=nano.specs）。
 *   这种 libc 下 printf 不走 fputc，而是：
 *       printf -> newlib stdio -> _write()        （Core/Src/syscalls.c）
 *                              -> __io_putchar()  逐字节输出
 *   而 syscalls.c 里 __io_putchar 只是一个 weak 声明、谁都没实现，
 *   链接器就把那次调用抹成了 NOP —— 所以"一个字都打不出来"。
 *   必须实现 __io_putchar（CubeMX 的标准重定向方式）才会打印。
 * ============================================================ */
int __io_putchar(int ch) {
  HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

/* scanf/getchar 用（syscalls.c 的 _read 会调它），不需要可以删 */
int __io_getchar(void) {
  uint8_t ch = 0;
  HAL_UART_Receive(&huart5, &ch, 1, HAL_MAX_DELAY);
  return (int)ch;
}

/* ============================================================
 *   fputc：只有 Keil/MicroLib 或 newlib 全量库的某些配置才走它，
 *   newlib-nano 下 printf 用不到（会被 --gc-sections 收掉）。
 *   留着无害，作为换工具链时的兼容写法。
 * ============================================================ */
int fputc(int ch, FILE *f) {
  (void)f;
  HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
