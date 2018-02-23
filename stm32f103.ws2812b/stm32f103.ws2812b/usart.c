#include <usart.h>


/*
 * ��������� usart
 */
void usart_deinit(void)
{
	USART_Cmd(USART1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
	USART_DeInit(USART1);
}


/*
* Function Name  : uart_send
* Description    : �������� ������ ������ �� UART.
* Input          : - pucBuffer: buffers to be printed.
*                : - ulCount  : buffer's length
*/
void uart_send(const char *pucBuffer, uint32_t ulCount)
{	
	
	uint16_t wait_flag = 0; // ��� �������� ����� ���� ���������� ������� � ������ ��������� Blutooth
	
	while (ulCount--)
	{
		USART_SendData(USART1, *pucBuffer++);
		
		/* ���� ��������� ��������, �� �� ������ �������� */
		while (!usart_tx_poll() &&
			   (wait_flag < UART_SEND_TIMEOUT))
		{
			wait_flag++;
		}
	}
}

/*
 * ��������� ������ ���� (���� ����� �����)
 * 
 * @param c      �������� ����
 */
void usart_sync_read(uint8_t * c)
{  
	while (!usart_rx_poll());
	
	*c = USART_ReceiveData(USART1);
}

/*
 * ��������� ������� ����� uart
 * 
 * @return true, ���� � �������� ������ ���� ����
 */
bool usart_rx_poll(void)
{
	return (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET);
}

/*
 * ��������� �������� ����� uart
 * 
 * @return true, ���� �������� ����� ������
 */
bool usart_tx_poll(void)
{
	return (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != RESET);
}

