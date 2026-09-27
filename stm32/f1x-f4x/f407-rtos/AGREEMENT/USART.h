#ifndef __USART_H__
#define __USART_H__


/* ================== 全局变量声明（extern） ================== */
extern volatile uint8_t USART_flag;      // USART1 收到的指令标志
extern uint8_t  Serial_RxPacket[4];      // USART2 接收的数据包
extern uint8_t  Serial_RxFlag;           // USART2 收到完整包标志
extern uint16_t Serial_RxCrc;            // 接收到的 CRC-16

/* ================== 函数声明 ================== */

/* CRC-16/MODBUS */
uint16_t Serial_CRC16(uint8_t *Data, uint16_t Length);

/* USART1：接上位机 */
void USART1_init(void);
void USART1_SendData(uint8_t data);
void USART1_SendString(char *string);
void USART1_Printf(char *format, ...);

/* USART2：接 F103，收 CRC-16 包 */
void USART2_init(void);

#endif