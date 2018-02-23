#include <usart.h>


/*
 * выключает usart
 */
void usart_deinit(void)
{
	USART_Cmd(USART1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
	USART_DeInit(USART1);
}


/*
* Function Name  : uart_send
* Description    : Отсылает строку данных по UART.
* Input          : - pucBuffer: buffers to be printed.
*                : - ulCount  : buffer's length
*/
void uart_send(const char *pucBuffer, uint32_t ulCount)
{	
	
	uint16_t wait_flag = 0; // без таймаута может быть постоянный простой в случае зависания Blutooth
	
	while (ulCount--)
	{
		USART_SendData(USART1, *pucBuffer++);
		
		/* ждем окончания передачи, но не дольше таймаута */
		while (!usart_tx_poll() &&
			   (wait_flag < UART_SEND_TIMEOUT))
		{
			wait_flag++;
		}
	}
}

/*
 * синхронно читает байт (ждет прием байта)
 * 
 * @param c      принятый байт
 */
void usart_sync_read(uint8_t * c)
{  
	while (!usart_rx_poll());
	
	*c = USART_ReceiveData(USART1);
}

/*
 * проверяет входной буфер uart
 * 
 * @return true, если в приемном буфере есть байт
 */
bool usart_rx_poll(void)
{
	return (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET);
}

/*
 * проверяет выходной буфер uart
 * 
 * @return true, если выходной буфер пустой
 */
bool usart_tx_poll(void)
{
	return (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != RESET);
}

