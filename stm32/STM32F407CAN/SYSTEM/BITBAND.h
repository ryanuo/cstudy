#ifndef __BITBAND__H_
#define __BITBAND__H_
/*
bit_word_addr = bit_band_base+(byte_offsetx32)+(bit_numberx4)
bit_word_addr 代表别名区域中将映射到目标位的字的地址
bit_band_base 代表别名区域的起始地址
byte_offset 代表目标位所在位段区域中的字节编号
bit_number 代表目标位的位位置 (0-7)
*/
#define BITBAND(addr,bitnum)    (0x42000000 +(addr-0x40000000)*32 + (bitnum*4))
#define MEMADDR(addr)            (*((uint32_t*)addr))
#define BITBUND(addr,bitnum)    MEMADDR(BITBAND(addr,bitnum))

#define GPIOA_IDR_ADDR          GPIOA_BASE+0X10
#define GPIOA_ODR_ADDR          GPIOA_BASE+0X14
#define GPIOB_IDR_ADDR          GPIOB_BASE+0X10
#define GPIOB_ODR_ADDR          GPIOB_BASE+0X14
#define GPIOF_IDR_ADDR          GPIOF_BASE+0X10
#define GPIOF_ODR_ADDR          GPIOF_BASE+0X14
#define GPIOG_IDR_ADDR          GPIOG_BASE+0X10
#define GPIOG_ODR_ADDR          GPIOG_BASE+0X14


#define PAin(n)                 BITBUND(GPIOA_IDR_ADDR,n)
#define PAout(n) 							  BITBUND(GPIOA_ODR_ADDR,n)
#define PBin(n)                 BITBUND(GPIOB_IDR_ADDR,n)
#define PBout(n) 							  BITBUND(GPIOB_ODR_ADDR,n)
#define PFin(n)                 BITBUND(GPIOF_IDR_ADDR,n)
#define PFout(n) 							  BITBUND(GPIOF_ODR_ADDR,n)
#define PGin(n)                 BITBUND(GPIOG_IDR_ADDR,n)
#define PGout(n) 							  BITBUND(GPIOG_ODR_ADDR,n)
#endif


