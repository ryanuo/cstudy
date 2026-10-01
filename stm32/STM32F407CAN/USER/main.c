#include "stm32f4xx.h"                  // Device header
#include "LED.h"
#include "BEEP.h"
#include "KEY.h"
#include "LIGHT.h"
#include "DELAY.h"
#include "EXTI.h"
#include "TIM.h"
#include "PWM.h"
#include "IWDG.h"
#include "WWDG.h"
#include "FLASH.h"
#include "USART.h"
#include <stdio.h>
#include "ADC.h"
#include "DAC.h"
#include "PWR.h"
#include "ONEWIREBUS.h"
#include "BITBAND.h"
#include "SPI.h"
#include "W25Q128.h"
#include "IIC.h"
#include "AT24C02.h"
#include "CAN.h"

int main(void) 
{
  DELAY_ms(1000);
	DELAY_ms(1000);
	DELAY_ms(1000);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_init();
	BEEP_init();
	//KEY_init();
	EXTI_init();
	LIGHT_init();
	TIM7_init();
	PWM_init();
	//IWDG_init();
  //WWDG_init();
  USART1_init();
  ADC1PA5_init();
	DACOUT1_init();
	PWR_init();
	DHT11_init();
  SPI1SOFTWARE_init();
	IICSOFTWARE_init();
  CAN_init();
	uint8_t senddata = 0X78;
	while(1)
	{
    CAN_sendmessage(0X123,&senddata,1);
//		if(CAN_rxflag == 1)
//		{
//		  CAN_rxflag = 0;
//			printf("%d\r\n",CAN_rxbuff[0]);
//		}
		DELAY_ms(500);
	}
}

