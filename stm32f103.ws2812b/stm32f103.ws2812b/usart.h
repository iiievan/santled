#ifndef _USART_H
#define _USART_H

#define __USE_C99_MATH	// ��� ���� ����� ��� bool ��� ���������.
#include <stdbool.h>	// ��� ���� ����� ��� bool ��� ���������.
#include <stm32f10x_conf.h>
#include <stm32f10x.h>


#define UART_SEND_TIMEOUT 0x0FFF    // ����� �� ���� ����������� �������� �� USART

void USART1_IRQHandler(void);
void usart_deinit(void);
void uart_send(const char *pucBuffer, uint32_t ulCount);
void usart_sync_read(uint8_t * c);
bool usart_rx_poll(void);
bool usart_tx_poll(void);

#endif // !_USART_H

