#include "stm32f4xx.h"                  // Device header
#include "FLASH.h"
#include "DELAY.h"
//任何FLASH遵循三个函数读写擦
void FLASH_write(uint32_t data)//字节，半字，字，双字
{
	//FLASH解锁
		FLASH_Unlock();
	//2、清除FLASH标志位
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_OPERR|FLASH_FLAG_WRPERR|FLASH_FLAG_PGAERR|FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR|FLASH_FLAG_RDERR);
	//3、扇区擦除：默认的值0XFF，只能1变0，不能0变1
	FLASH_EraseSector(FLASH_Sector_7,VoltageRange_3);
	//4、数据写入
	FLASH_ProgramWord(0X08060000,data);
	//5、FLASH上锁
	FLASH_Lock();
	DELAY_ms(100);
}

void FLASH_erase(void)
{
	//FLASH解锁
		FLASH_Unlock();
	//2、清除FLASH标志位
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_OPERR|FLASH_FLAG_WRPERR|FLASH_FLAG_PGAERR|FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR|FLASH_FLAG_RDERR);
	//3、扇区擦除：默认的值0XFF，只能1变0，不能0变1
	FLASH_EraseSector(FLASH_Sector_7,VoltageRange_3);
	//4、FLASH上锁
	FLASH_Lock();
	DELAY_ms(100);
}

uint32_t FLASH_read(uint32_t address)
{
 return *((uint32_t*)address);
}
