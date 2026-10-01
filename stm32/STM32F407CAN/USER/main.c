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
#define CAN_ROLE_SENDER   0
#define CAN_DEBUG_PRINTF  1
#define CAN_TX_PERIOD_MS  500
#define CAN_TEST_ID       0x123
#define CAN_PIN_SNIFF     1     // 1 = 每秒数一次引脚跳变数（看信号到底进没进板子）
#define CAN_TX_PIN_TEST   1     // 0 = 关；1 = 开机+每约5秒把 CAN_TX(PD1) 当普通 IO 翻转，数 CAN_RX(PD0) 跟不跟（收发器→PD0 这段通不通）
#define CAN_DBG_VER       7     // 固件版本号：开机横幅里会打出来，用来确认板上烧的是哪一版

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
#endif /* CAN_DEBUG_PRINTF */

#if CAN_PIN_SNIFF == 1
/*---------------------------------------------------------------
 * 引脚嗅探：约 1 秒内数 6 个脚的跳变次数，打到串口
 *
 * 为什么有用：发送端现在因为没人 ACK 在【不停重传】，总线上的波形
 * 是一直有的。所以：
 *   哪个脚在跳  = 收发器的 RXD 实际接在那个脚上（顺便查出接错脚）
 *   全都不跳    = 信号根本没进板子（收发器没供电/没工作、线断）
 *   PD1 在跳    = MCU 确实在往收发器送数据（发送端应该狂跳）
 *   PD0 在跳    = 收发器把总线状态回读给了 MCU（发送端也应有）
 *
 * PD0/PD1 是 CAN 复用脚，AF 模式下 GPIOD->IDR 照样反映引脚真实电平；
 * PA11/PA12 复位后是输入模式；PB8/PB9 被软件 IIC 配成了 GPIO，都能读。
 *---------------------------------------------------------------*/
static void CAN_pin_sniff(void)
{
  uint32_t c0 = 0,c1 = 0,c8 = 0,c9 = 0,ca = 0,cb = 0;
  uint32_t pd = GPIOD->IDR,pb = GPIOB->IDR,pa = GPIOA->IDR;
  volatile uint32_t n;
  for(n = 0;n < 8000000U;n++)//约 1 秒
  {
    uint32_t d = GPIOD->IDR,b = GPIOB->IDR,a = GPIOA->IDR;
    if((d ^ pd) & 0x0001U) c0++;
    if((d ^ pd) & 0x0002U) c1++;
    if((b ^ pb) & 0x0100U) c8++;
    if((b ^ pb) & 0x0200U) c9++;
    if((a ^ pa) & 0x0800U) ca++;
    if((a ^ pa) & 0x1000U) cb++;
    pd = d;pb = b;pa = a;
  }
  printf("SN PD0=%u PD1=%u\r\n",(unsigned)c0,(unsigned)c1);
  printf("SN PB8=%u PB9=%u\r\n",(unsigned)c8,(unsigned)c9);
  printf("SN PA11=%u PA12=%u\r\n",(unsigned)ca,(unsigned)cb);
}
#endif

#if CAN_TX_PIN_TEST == 1
static uint16_t s_txpd_cnt = 0;//每 5 个自检槽重跑一次通路测试

/*---------------------------------------------------------------
 * 数 CAN_RX(PD0) 在 iters 次采样里跳变了几次
 *---------------------------------------------------------------*/
static uint32_t CAN_count_rx(uint32_t iters)
{
  uint32_t cnt = 0,prev = GPIOD->IDR & 0x0001U,k;
  for(k = 0;k < iters;k++)
  {
    uint32_t v = GPIOD->IDR & 0x0001U;
    if(v != prev)
    {
      cnt++;
      prev = v;
    }
  }
  return cnt;
}

/*---------------------------------------------------------------
 * 开机自检：把 CAN_TX(PD1) 临时当普通推挽输出，慢速手动翻转，
 * 同时数 CAN_RX(PD0) 的跳变。一次就能判断接收端本地这一段：
 *   TXPD PD0 > 0  = 收发器在工作，而且它的 RXD 确实接到了 PD0
 *                   （那收不到就只能是两板之间的总线线/共地问题）
 *   TXPD PD0 = 0  = 收发器没供电/没工作，或 RXD 到 PD0 这段断
 * 测完自动把 PD1 恢复成 CAN1 复用（AF9），不影响后面的测试。
 *---------------------------------------------------------------*/

/*---------------------------------------------------------------
 * 六脚电平快照：PD0 PD1 PB8 PB9 PA11 PA12（各 1 位，依次打印）
 *---------------------------------------------------------------*/
static uint32_t CAN_levels6(void)
{
  uint32_t v = 0;
  if(GPIOD->IDR & 0x0001U) v |= 0x20U;//PD0
  if(GPIOD->IDR & 0x0002U) v |= 0x10U;//PD1
  if(GPIOB->IDR & 0x0100U) v |= 0x08U;//PB8
  if(GPIOB->IDR & 0x0200U) v |= 0x04U;//PB9
  if(GPIOA->IDR & 0x0800U) v |= 0x02U;//PA11
  if(GPIOA->IDR & 0x1000U) v |= 0x01U;//PA12
  return v;
}

static void CAN_print_levels(const char *tag,uint32_t v)
{
  printf("PROBE %s %u %u %u %u %u %u\r\n",tag,
         (unsigned)((v >> 5) & 1U),(unsigned)((v >> 4) & 1U),
         (unsigned)((v >> 3) & 1U),(unsigned)((v >> 2) & 1U),
         (unsigned)((v >> 1) & 1U),(unsigned)(v & 1U));
}

static void CAN_tx_pin_test(void)
{
  uint32_t i,edges = 0;
  GPIO_InitTypeDef gi;
  gi.GPIO_Pin = GPIO_Pin_1;
  gi.GPIO_Mode = GPIO_Mode_OUT;
  gi.GPIO_OType = GPIO_OType_PP;
  gi.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gi.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD,&gi);//PD1 先变成普通输出

  for(i = 0;i < 500U;i++)
  {
    GPIO_ResetBits(GPIOD,GPIO_Pin_1);//CAN_TX 拉低 -> 收发器把总线拉成显性
    edges += CAN_count_rx(3000U);
    GPIO_SetBits(GPIOD,GPIO_Pin_1);//放开总线
    edges += CAN_count_rx(3000U);
  }

  /* 电平探针：PD1 保持高 60 ms / 拉低 60 ms，各读一次六个候选脚的静态电平。
     PD1 高时 PD0=1、PD1 低时 PD0=0 => 收发器在驱动总线并把总线状态回读；
     PD0 两次都一样(例如都是 1) => PD0 没被收发器驱动(悬空/芯片没供电)。*/
  GPIO_SetBits(GPIOD,GPIO_Pin_1);
  DELAY_ms(60);
  CAN_print_levels("hi",CAN_levels6());
  GPIO_ResetBits(GPIOD,GPIO_Pin_1);
  DELAY_ms(60);
  CAN_print_levels("lo",CAN_levels6());

  gi.GPIO_Mode = GPIO_Mode_AF;//恢复 CAN1 复用
  GPIO_Init(GPIOD,&gi);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_CAN1);

  printf("TXPD PD0=%u", (unsigned)edges);
  if(edges > 0U)
  {
    printf(" (RX path OK)");
  }
  else
  {
    printf(" (RX path DEAD)");
  }
  printf("\r\n");
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
  printf("\r\n==== CAN %s v%u ====  ID=0x%03X\r\n",
         (CAN_ROLE_SENDER == 1) ? "SENDER" : "RECEIVER",
         (unsigned)CAN_DBG_VER,
         (unsigned)CAN_TEST_ID);
  CAN_print_diag();
#endif
#if CAN_TX_PIN_TEST == 1
  CAN_tx_pin_test();//开机自检一次：手动驱动 CAN_TX，看 CAN_RX 跟不跟
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
#if CAN_PIN_SNIFF == 1
			CAN_pin_sniff();//紧随其后数 1 秒引脚跳变
#endif
#if CAN_TX_PIN_TEST == 1
			if(++s_txpd_cnt >= 8U)//发送端每约 4 秒也跑一次，方便两块板 A/B 对比
			{
				s_txpd_cnt = 0;
				CAN_tx_pin_test();
			}
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
#if CAN_PIN_SNIFF == 1
			CAN_pin_sniff();//紧随其后数 1 秒引脚跳变
#endif
#if CAN_TX_PIN_TEST == 1
			if(++s_txpd_cnt >= 5U)//每约 5 秒重跑一次收发通路自检
			{
				s_txpd_cnt = 0;
				CAN_tx_pin_test();
			}
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
