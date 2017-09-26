
/* 0xWS2812 16-Channel WS2812 interface library
 * 
 * Copyright (c) 2014 Elia Ritterbusch, http://eliaselectronics.com
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <main.h>


uint16_t WS2812_IO_High = 0xFFFF;
uint16_t WS2812_IO_Low = 0x0000;

volatile uint8_t TIM2_overflows = 0;

static uint32_t usart_recieve = 0; // ���������� ������ �� USART  

/* simple delay counter to waste time, don't rely on for accurate timing */
void Delay(__IO uint32_t nCount)
{
	while (nCount--) {
	}
}

void GPIO_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	// GPIOA Periph clock enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	// ����������� ����� GPIOA ��� ���������� WS2812
	GPIO_InitStructure.GPIO_Pin = 0x00FF;	// PA.00...PA.07
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// ��� ��� ��� ���������� HC-06, ���� �� ������������.
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 

	// ������������� USART1 Tx (���� PA.09) ��� ������������ (push-pull) ��� 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// ������������� USART1 Rx (PA.10) ��� ���� ��� �������� 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void TIM2_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	uint16_t PrescalerValue;
	
	// TIM2 Periph clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	PrescalerValue = (uint16_t)(SystemCoreClock / 72000000) - 1;
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 29; // 800kHz
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM2, DISABLE);

	/* Timing Mode configuration: Channel 1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 8;
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);	

	/* Timing Mode configuration: Channel 2 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 17;
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Disable);
	
	/* configure TIM2 interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void DMA_init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	// TIM2 Update event
	/* DMA1 Channel2 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&GPIOA->ODR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)WS2812_IO_High;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);
	
	// TIM2 CC1 event
	/* DMA1 Channel5 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&GPIOA->ODR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)WS2812_IO_framedata;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);
	
	// TIM2 CC2 event
	/* DMA1 Channel7 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&GPIOA->ODR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)WS2812_IO_Low;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);

	/* configure DMA1 Channel7 interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* enable DMA1 Channel7 transfer complete interrupt */
	DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);
}

void USART_init(void)
{
	const char welcome_str[] = " Welcome to Bluetooth!\r\n";
	
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* ����������� USART1 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* USART1 ��������������� ��� ���:
	        - BaudRate = 38400 baud
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
	
	USART_InitStructure.USART_BaudRate = 38400;		// �������� ��������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
	
    /* ����������� ���������� �� USART1 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

	/* �������� USART1 */
	USART_Cmd(USART1, ENABLE);
	/* �������� ���������� �� USART1 */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	/* ����������� � ��������. */
	UARTSend(welcome_str, sizeof(welcome_str));
}



/* DMA1 Channel7 Interrupt Handler gets executed once the complete framebuffer has been transmitted to the LEDs */
void DMA1_Channel7_IRQHandler(void)
{
	// clear DMA7 transfer complete interrupt flag
	DMA_ClearITPendingBit(DMA1_IT_TC7);	
	// enable TIM2 Update interrupt to append 50us dead period
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	// disable the DMA channels
	DMA_Cmd(DMA1_Channel2, DISABLE);	
	DMA_Cmd(DMA1_Channel5, DISABLE);
	DMA_Cmd(DMA1_Channel7, DISABLE);
	// IMPORTANT: disable the DMA requests, too!
	TIM_DMACmd(TIM2, TIM_DMA_CC1, DISABLE);
	TIM_DMACmd(TIM2, TIM_DMA_CC2, DISABLE);
	TIM_DMACmd(TIM2, TIM_DMA_Update, DISABLE);
	
}

/* TIM2 Interrupt Handler gets executed on every TIM2 Update if enabled */
void TIM2_IRQHandler(void)
{
	// Clear TIM2 Interrupt Flag
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	
	/* check if certain number of overflows has occured yet 
	 * this ISR is used to guarantee a 50us dead time on the data lines
	 * before another frame is transmitted */
	if (TIM2_overflows < (uint8_t)WS2812_DEADPERIOD)
	{
		// count the number of occured overflows
		TIM2_overflows++;
	}
	else
	{
		// clear the number of overflows
		TIM2_overflows = 0;	
		// stop TIM2 now because dead period has been reached
		TIM_Cmd(TIM2, DISABLE);
		/* disable the TIM2 Update interrupt again 
		 * so it doesn't occur while transmitting data */
		TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
		// finally indicate that the data frame has been transmitted
		WS2812_TC = 1; 	
	}
}

/*
 *
 *  ������� ���������� ���������� USARTx.
 */

void USART1_IRQHandler(void)
{
	if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET)
	{
		usart_recieve = USART_ReceiveData(USART1);
		
		if (usart_recieve == '1') {
			GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);		// ������������� '1' �� 8 ����
			UARTSend("LED ON\r\n", sizeof("LED ON\r\n"));	// ������� ������� � UART
		}
		else if (usart_recieve == '0') {
			GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);    // ������������� '0' �� 8 ����
			UARTSend("LED OFF\r\n", sizeof("LED OFF\r\n"));
		}
	}
}

/*
* Function Name  : UARTSend
* Description    : �������� ������ ������ �� UART.
* Input          : - pucBuffer: buffers to be printed.
*                : - ulCount  : buffer's length
*/
void UARTSend(const char *pucBuffer, uint32_t ulCount)
{
    //
    // Loop while there are more characters to send.
    //
	while (ulCount--)
	{
		USART_SendData(USART1, (uint16_t) *pucBuffer++);
		/* Loop until the end of transmission */
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
		{
		}
	}
}



int main(void) 
{	
	static uint8_t i,j;
	static uint32_t rgb_after_correct;
	static uint32_t rng_val;
	static uint8_t  red_coeff,
			      green_coeff,
			       blue_coeff;
	static rgb_operation red_op,
						 green_op,
						 blue_op;
	
	GPIO_init();
	DMA_init();
	TIM2_init();		// ������ ��� ������ � WS2812
	adc_rng_init();	    // ��� ��� ��������� ���������� �����.
	void USART_init();	// ����������� USART1 ��� ������ � HC-06
	
	while (1) {
		// set two pixels (columns) in the defined row (channel 0) to the
		// color values defined in the colors array
	//	for (i = 0; i < NUM_OF_FRAMES; i++)
	//	{
			// �������� ����� �� ������������ ��� ������ �� Bluetooth
			red_coeff = put_rgb_mask(usart_recieve, RED);	
			green_coeff = put_rgb_mask(usart_recieve, GREEN);	
			blue_coeff = put_rgb_mask(usart_recieve, BLUE);	
			// �������� ��������, ������� ����� �������� � ������ � �����.
		    red_op = eject_operation(usart_recieve, RED);
		    green_op = eject_operation(usart_recieve, GREEN);
		    blue_op = eject_operation(usart_recieve, BLUE);
		

	
		
			srand(adc_rng_get());   // ����� ��� ��������� ���������� �����.
			
		    i = rand() % 24;	    // ���������  ���� �� 24-�.
			
			for (j = 0; j < NUMOFLEDS; j++)
			{
				// wait until the last frame was transmitted
				while (!WS2812_TC);
			
				// ������������ ������� ����� ������.
			//  rgb_after_correct = tone_correction_func(frames[i][j], 0xFF, ADD, 0x57, SUB, 0x37, SUB);
				
				// ������������ ������� ����� ������ ���, ��� �� Bluetooth
				// ������� �� Bluetooth, ������� ������ ������ ������� �� ������: 0x40FF5737
				rgb_after_correct = tone_correction_func(frames[i][j], red_coeff, red_op, green_coeff, green_op, blue_coeff, blue_op);
				
				// this approach sets each pixel individually
				WS2812_framedata_setPixel(4, j, rgb_after_correct);
				WS2812_framedata_setPixel(5, j, rgb_after_correct);
				WS2812_framedata_setPixel(6, j, rgb_after_correct);
				WS2812_framedata_setPixel(7, j, rgb_after_correct);
			}
		
			WS2812_sendbuf(BUFFERSIZE);
			Delay(400000);			
	   //}				
	}
}




