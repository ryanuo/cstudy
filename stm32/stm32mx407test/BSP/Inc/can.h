#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "main.h"

void    BSP_CAN_Init(void);
void    MyCAN_Transmit(uint32_t ID, uint8_t Length, uint8_t *Data);
uint8_t MyCAN_ReceiveFlag(void);
void    MyCAN_Receive(uint32_t *ID, uint8_t *Length, uint8_t *Data);

/* 接收回调，用户重写 */
void MyCAN_OnRx(uint32_t ID, uint8_t Length, uint8_t *Data);

/*------------------------------------------------------------------
 * 自检：把 CAN1 的寄存器原样读回来，不接调试器也能定位问题
 *
 *   rf0r   FMP0[1:0]  = FIFO0 里积压的帧数（一直是 0 = 一帧都没进来）
 *   esr    LEC[6:4]   = 最近一次错误类型（0=无错，越大越严重）
 *          TEC[23:16] = 发送错误计数    REC[31:24] = 接收错误计数
 *   msr    INAK=bit0、SLAK=bit1：两个都为 0 才是正常运行
 *          （复位值 0x00000C02 = SLAK 睡着；INAK=1 表示停在初始化模式）
 *   tsr    TXOK0[0]=1 = 邮箱0 发送成功；TERR0[15]=1 发送失败；TME0[26]=1 邮箱空
 *   fmr    FINIT[0]   = 过滤器初始化位；CAN2SB[13:8] = CAN2 起始 bank
 *          （F407 是双 CAN：CAN2SB=0 表示 28 个过滤 bank 全归 CAN2，
 *            CAN1 名下没有 bank，收到什么都会被丢掉）
 *   fa1r   哪些过滤 bank 是激活的（bit0 对应 bank0，即本工程配的那一个）
 *   fm1r   0 = 掩码模式，1 = 列表模式
 *   fs1r   1 = 32 位，0 = 16 位
 *   ffa1r  0 = 该 bank 挂 FIFO0，1 = 挂 FIFO1
 *   fr1    bank0 的 ID  寄存器（32 位模式下 = ID 的位）
 *   fr2    bank0 的掩码寄存器（掩码位 = 1 表示该位必须和 ID 相同）
 *   ier    bit1 = 1 才是 FIFO0 消息中断使能（=0 就永远不进回调）
 *   gpiod_moder / gpiod_afr0 / gpiod_idr = PD0/PD1 的 GPIO 配置与实时电平
 *          MODER 低 4 位：PD0=bits[1:0]、PD1=bits[3:2]，AF 模式 = 0b10 → 期望 0x0000000A
 *          AFR0  低 8 位：PD0=bits[3:0]、PD1=bits[7:4]，AF9      → 期望 0x00000099
 *          IDR   低 2 位：静默时 PD0(隐性)、PD1 都应为高          → 期望 0x00000003
 *----------------------------------------------------------------*/
typedef struct {
  uint32_t rf0r;
  uint32_t esr;
  uint32_t msr;
  uint32_t tsr;
  uint32_t fmr;
  uint32_t fa1r;
  uint32_t fm1r;
  uint32_t fs1r;
  uint32_t ffa1r;
  uint32_t fr1;
  uint32_t fr2;
  uint32_t ier;         /* CAN_IER */
  uint32_t gpiod_moder; /* GPIOD->MODER */
  uint32_t gpiod_afr0;  /* GPIOD->AFR[0] */
  uint32_t gpiod_idr;   /* GPIOD->IDR */
  uint32_t rx_irq_cnt; /* RX FIFO0 中断进入次数（不论 ID 是否匹配） */
  uint32_t rx_ok_cnt;  /* 从 FIFO0 成功取出的帧数 */
  uint32_t tx_req_cnt; /* 调用发送的次数 */
  uint32_t err;        /* hcan1.ErrorCode 汇总的错误标志 */
} CAN_Diag_t;

void CAN_Diag_Read(CAN_Diag_t *diag);

/* 计数器（在中断里累加），可以直接拿来点灯/判活 */
extern volatile uint32_t g_can_rx_irq_cnt; /* 进 RX FIFO0 中断的次数（不论 ID） */
extern volatile uint32_t g_can_rx_ok_cnt;  /* 成功从 FIFO0 取出的帧数 */
extern volatile uint32_t g_can_tx_req_cnt; /* 调用发送的次数 */

#endif