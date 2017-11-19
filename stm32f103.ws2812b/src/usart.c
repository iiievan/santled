#include <usart.h>


void usart_init(void)
{
	const char welcome_str[] = " Welcome to Bluetooth!\r\n";
	
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* затактируем USART1 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* USART1 сконфигурирован вот так:
	        - BaudRate = 115200 baud
	        - Word Length = 8 Bits
	        - One Stop Bit
	        - No parity
	        - Hardware flow control disabled (RTS and CTS signals)
	        - Receive and transmit enabled
	        - USART Clock disabled
	        - USART CPOL: Clock is active low
	        - USART CPHA: Data is captured on the middle
	        - USART LastBit: The clock pulse of the last data bit is not output to
	                         the SCLK pin
	*/
	
	USART_InitStructure.USART_BaudRate = 115200;		// Скорость передачи
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
	
    /* Настраиваем прерывание от USART1 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

	/* Включаем USART1 */
	USART_Cmd(USART1, ENABLE);
	/* включаем прерывание по USART1 */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	/* приветствие в терминал. */
	uart_send(welcome_str, sizeof(welcome_str));
}

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

