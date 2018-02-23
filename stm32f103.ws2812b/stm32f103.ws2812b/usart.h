#ifndef _USART_H
#define _USART_H

#define __USE_C99_MATH	// для того чтобы тип bool был определен.
#include <stdbool.h>	// для того чтобы тип bool был определен.
#include <stm32f10x_conf.h>
#include <stm32f10x.h>


#define UART_SEND_TIMEOUT 0x0FFF    // чтобы не было бесконечной отправки по USART

void USART1_IRQHandler(void);
void usart_deinit(void);
void uart_send(const char *pucBuffer, uint32_t ulCount);
void usart_sync_read(uint8_t * c);
bool usart_rx_poll(void);
bool usart_tx_poll(void);

#endif // !_USART_H

