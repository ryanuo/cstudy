#include "can.h"

extern CAN_HandleTypeDef hcan1;

/*------------------------------------------------------------------
 * 自检计数器（volatile：在中断里改写）
 *----------------------------------------------------------------*/
volatile uint32_t g_can_rx_irq_cnt = 0; /* RX FIFO0 中断进入次数 */
volatile uint32_t g_can_rx_ok_cnt = 0;  /* 从 FIFO0 成功取出的帧数 */
volatile uint32_t g_can_tx_req_cnt = 0; /* 调用发送的次数 */

/*------------------------------------------------------------------
 * 初始化：过滤器 + 启动 + 开中断
 *----------------------------------------------------------------*/
void BSP_CAN_Init(void) {
  CAN_FilterTypeDef sFilterConfig = {0};

  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

  /* 32位掩码模式，全接收：掩码 = 0 才是"所有位都不关心"。
     掩码位 = 1 表示"该位必须和 ID 相同"，写 0xFFFF 会把 ID 卡死成 0x000 */
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000; // 关键改动
  sFilterConfig.FilterMaskIdLow = 0x0000;  // 关键改动

  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;

  /* 单CAN芯片此字段无意义；双CAN芯片按手册设为从过滤器起始bank。
     F407 是双CAN：14 表示 bank0~13 归 CAN1，写 0 等于 CAN1 没有 bank。 */
  sFilterConfig.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_CAN_Start(&hcan1) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) !=
      HAL_OK) {
    Error_Handler();
  }
}
/*------------------------------------------------------------------
 * 发送
 *----------------------------------------------------------------*/
void MyCAN_Transmit(uint32_t ID, uint8_t Length, uint8_t *Data) {
  CAN_TxHeaderTypeDef TxHeader;
  uint32_t TxMailbox;

  g_can_tx_req_cnt++;

  if (Length > 8)
    Length = 8;

  TxHeader.StdId = ID & 0x7FF;
  TxHeader.ExtId = 0;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.DLC = Length;
  TxHeader.TransmitGlobalTime = DISABLE;

  if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0) {
    return;
  }

  HAL_CAN_AddTxMessage(&hcan1, &TxHeader, Data, &TxMailbox);
}

/*------------------------------------------------------------------
 * 查询是否有报文
 *----------------------------------------------------------------*/
uint8_t MyCAN_ReceiveFlag(void) {
  return (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) ? 1 : 0;
}

/*------------------------------------------------------------------
 * 主动读一帧（轮询方式）
 *----------------------------------------------------------------*/
void MyCAN_Receive(uint32_t *ID, uint8_t *Length, uint8_t *Data) {
  CAN_RxHeaderTypeDef RxHeader;
  uint8_t RxData[8];

  if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
    return;
  }

  *ID = (RxHeader.IDE == CAN_ID_STD) ? RxHeader.StdId : RxHeader.ExtId;

  if (RxHeader.RTR == CAN_RTR_DATA) {
    *Length = RxHeader.DLC;
    for (uint8_t i = 0; i < *Length; i++) {
      Data[i] = RxData[i];
    }
  }
}

/*------------------------------------------------------------------
 * 自检：把 CAN1 的寄存器原样读回来（只读，不影响收发）
 *----------------------------------------------------------------*/
void CAN_Diag_Read(CAN_Diag_t *diag) {
  if (diag == 0) {
    return;
  }

  diag->rf0r = CAN1->RF0R;
  diag->esr = CAN1->ESR;
  diag->msr = CAN1->MSR;
  diag->tsr = CAN1->TSR;

  diag->fmr = CAN1->FMR;
  diag->fa1r = CAN1->FA1R;
  diag->fm1r = CAN1->FM1R;
  diag->fs1r = CAN1->FS1R;
  diag->ffa1r = CAN1->FFA1R;
  diag->fr1 = CAN1->sFilterRegister[0].FR1;
  diag->fr2 = CAN1->sFilterRegister[0].FR2;

  diag->ier = CAN1->IER;
  diag->gpiod_moder = GPIOD->MODER;
  diag->gpiod_afr0 = GPIOD->AFR[0];
  diag->gpiod_idr = GPIOD->IDR;

  diag->rx_irq_cnt = g_can_rx_irq_cnt;
  diag->rx_ok_cnt = g_can_rx_ok_cnt;
  diag->tx_req_cnt = g_can_tx_req_cnt;
  diag->err = hcan1.ErrorCode;
}

/*------------------------------------------------------------------
 * HAL 中断回调 → 转发给用户
 *----------------------------------------------------------------*/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  CAN_RxHeaderTypeDef RxHeader;
  uint8_t RxData[8];

  g_can_rx_irq_cnt++; /* 进了中断 = FIFO0 里确实有帧 */

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
    return;
  }

  g_can_rx_ok_cnt++;

  uint32_t id = (RxHeader.IDE == CAN_ID_STD) ? RxHeader.StdId : RxHeader.ExtId;
  MyCAN_OnRx(id, RxHeader.DLC, RxData);
}

/* 弱定义，用户不重写也不报错 */
__weak void MyCAN_OnRx(uint32_t ID, uint8_t Length, uint8_t *Data) {
  (void)ID;
  (void)Length;
  (void)Data;
}
