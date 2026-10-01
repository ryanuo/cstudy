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

/* ==========================================================================
 * CAN 收发测试开关（烧录前只改这一段）
 *
 *   CAN_ROLE_SENDER   1 = 发送端：每 CAN_TX_PERIOD_MS 毫秒发一帧，只发不收
 *                     0 = 接收端：收到帧就打印，并把 LED2/LED3 点亮
 *   CAN_DEBUG_PRINTF  1 = 串口打印（USART1，9600 8N1，串口助手别设 115200）
 *                     0 = 关掉所有打印，只用灯看结果
 *   CAN_TEST_ID       测试用的标准 ID（现在的过滤器是全通过，任何 ID 都收）
 *
 * 灯（低电平点亮）：LED1=PF9  LED2=PF10  LED3=PE13  LED4=PE14
 * ========================================================================== */
#define CAN_ROLE_SENDER   1
#define CAN_DEBUG_PRINTF  1
#define CAN_TX_PERIOD_MS  500
#define CAN_TEST_ID       0x123

#if CAN_DEBUG_PRINTF == 1
/* 一行寄存器自检，直接送到串口：
 *   N   收到的帧数（在 CAN 中断里无条件累加，不看 ID）
 *   FMP FIFO0 里还积压几帧
 *   ESR LEC=bit6:4 最近一次错误(0 无错 / 3 没人 ACK / 6 CRC 或波特率不符)
 *       TEC=bit23:16 发送错误计数   REC=bit31:24 接收错误计数
 *   TSR bit0=TXOK0（发送成功过）  bit26=TME0（邮箱0 空着）
 *   MSR 正常运行 = 00000C00（INAK/SLAK 都为 0）
 */
static void CAN_print_diag(void)
{
  uint32_t esr = CAN1->ESR;
  printf("N=%u FMP=%u LEC=%u TEC=%u REC=%u\r\n",
         (unsigned)CAN_rx_irq_cnt,(unsigned)(CAN1->RF0R & 0x03U),
         (unsigned)((esr >> 4) & 0x07U),
         (unsigned)((esr >> 16) & 0xFFU),(unsigned)((esr >> 24) & 0xFFU));
  printf("ESR=%08X TSR=%08X\r\n",(unsigned)esr,(unsigned)CAN1->TSR);
  printf("TXOK0=%u TME0=%u MSR=%08X\r\n",
         (unsigned)((CAN1->TSR & CAN_TSR_TXOK0) != 0U ? 1U : 0U),
         (unsigned)((CAN1->TSR & CAN_TSR_TME0) != 0U ? 1U : 0U),
         (unsigned)CAN1->MSR);
}
#endif

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
#if CAN_DEBUG_PRINTF == 1
  printf("\r\n==== CAN %s ====  ID=0x%03X\r\n",
         (CAN_ROLE_SENDER == 1) ? "SENDER" : "RECEIVER",
         (unsigned)CAN_TEST_ID);
  CAN_print_diag();
#endif
#if CAN_ROLE_SENDER == 1
	uint8_t senddata = 0X78;//发送端才用得到
#endif
	uint16_t loop_cnt = 0;
	while(1)
	{
#if CAN_ROLE_SENDER == 1
		/* ==================== 发送端 ==================== */
		uint8_t mb = CAN_sendmessage(CAN_TEST_ID,&senddata,1);
#if CAN_DEBUG_PRINTF == 1
		if((CAN1->TSR & CAN_TSR_TXOK0) != 0U)//TXOK0(bit1)=1：这帧发出去并被人 ACK 了
		{
			LED4_on();
		}
		printf("TX d0=%02X mb=%u\r\n",(unsigned)senddata,(unsigned)mb);//mb=邮箱号 0/1/2；mb=4=三个邮箱全满(没人 ACK 在重传)
#endif
		(void)mb;//关掉串口打印时靠这句消掉 unused 警告
		senddata++;
		if(++loop_cnt >= 4U)//约 2 秒打一行寄存器自检
		{
			loop_cnt = 0;
#if CAN_DEBUG_PRINTF == 1
			CAN_print_diag();
#endif
		}
		DELAY_ms(CAN_TX_PERIOD_MS);
#else
		/* ==================== 接收端 ==================== */
		if(CAN_rxflag == 1)
		{
			CAN_rxflag = 0;
			LED2_on();
			LED3_on();
#if CAN_DEBUG_PRINTF == 1
			printf("RX id=%03X dlc=%u d0=%02X N=%u\r\n",
			       (unsigned)CAN_rxid,(unsigned)CAN_rxlength,
			       (unsigned)CAN_rxbuff[0],(unsigned)CAN_rx_irq_cnt);
#endif
		}
		if(CAN_rx_irq_cnt > 0U)//收到过帧就一直亮，不会被看漏
		{
			LED2_on();
			LED3_on();
		}
		if(++loop_cnt >= 100U)//约 1 秒打一行自检（收不到帧时靠它看 ESR/TSR）
		{
			loop_cnt = 0;
#if CAN_DEBUG_PRINTF == 1
			CAN_print_diag();
#endif
		}
		DELAY_ms(10);
#endif

		/* ========== 原来的测试代码（留着，随时改回来） ==========
		CAN_sendmessage(0X123,&senddata,1);
		if(CAN_rxflag == 1)
		{
			CAN_rxflag = 0;
			printf("%d\r\n",CAN_rxbuff[0]);
		}
		DELAY_ms(500);
		====================================================== */
	}
}
