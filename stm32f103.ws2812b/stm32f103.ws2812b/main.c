
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

volatile uint8_t WS2812_TC = 1;
volatile uint8_t TIM2_overflows = 0;

static uint32_t usart_recieve = 0; // получаемые данные из USART  

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
	
	// настраиваем ножки GPIOA для управления WS2812
	GPIO_InitStructure.GPIO_Pin = 0x00FF;	// PA.00...PA.07
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// это пин для управления HC-06, пока не задействован.
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 

	// Конфигурируем USART1 Tx (нога PA.09) как симметричный (push-pull) пин 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Конфигурируем USART1 Rx (PA.10) как вход без подтяжки 
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
	
	/* затактируем USART1 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* USART1 сконфигурирован вот так:
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
	
	USART_InitStructure.USART_BaudRate = 38400;		// Скорость передачи
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
	UARTSend(welcome_str, sizeof(welcome_str));
}

/* Transmit the frambuffer with buffersize number of bytes to the LEDs 
 * buffersize = (#LEDs / 16) * 24 */
void WS2812_sendbuf(uint32_t buffersize)
{		
	// transmission complete flag, indicate that transmission is taking place
	WS2812_TC = 0;
	
	// clear all relevant DMA flags
	DMA_ClearFlag(DMA1_FLAG_TC2 | DMA1_FLAG_HT2 | DMA1_FLAG_GL2 | DMA1_FLAG_TE2);
	DMA_ClearFlag(DMA1_FLAG_TC5 | DMA1_FLAG_HT5 | DMA1_FLAG_GL5 | DMA1_FLAG_TE5);
	DMA_ClearFlag(DMA1_FLAG_HT7 | DMA1_FLAG_GL7 | DMA1_FLAG_TE7);
	
	// configure the number of bytes to be transferred by the DMA controller
	DMA_SetCurrDataCounter(DMA1_Channel2, buffersize);
	DMA_SetCurrDataCounter(DMA1_Channel5, buffersize);
	DMA_SetCurrDataCounter(DMA1_Channel7, buffersize);
	
	// clear all TIM2 flags
	TIM2->SR = 0;
	
	// enable the corresponding DMA channels
	DMA_Cmd(DMA1_Channel2, ENABLE);
	DMA_Cmd(DMA1_Channel5, ENABLE);
	DMA_Cmd(DMA1_Channel7, ENABLE);
	
	// IMPORTANT: enable the TIM2 DMA requests AFTER enabling the DMA channels!
	TIM_DMACmd(TIM2, TIM_DMA_CC1, ENABLE);
	TIM_DMACmd(TIM2, TIM_DMA_CC2, ENABLE);
	TIM_DMACmd(TIM2, TIM_DMA_Update, ENABLE);
	
	// preload counter with 29 so TIM2 generates UEV directly to start DMA transfer
	TIM_SetCounter(TIM2, 29);
	
	// start TIM2
	TIM_Cmd(TIM2, ENABLE);
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
 *  Функция обработчик прерывания USARTx.
 */

void USART1_IRQHandler(void)
{
	if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET)
	{
		usart_recieve = USART_ReceiveData(USART1);
		if (usart_recieve == '1') {
			GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);		// Устанавливаем '1' на 8 ноге
			UARTSend("LED ON\r\n", sizeof("LED ON\r\n"));	// Выводим надпись в UART
		}
		else if (usart_recieve == '0') {
			GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);    // Устанавливаем '0' на 8 ноге
			UARTSend("LED OFF\r\n", sizeof("LED OFF\r\n"));
		}
	}
}

/*
* Function Name  : UARTSend
* Description    : Отсылает строку данных по UART.
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

/* This function sets the color of a single pixel in the framebuffer 
 * 
 * Arguments:
 * row = the channel number/LED strip the pixel is in from 0 to 15
 * column = the column/LED position in the LED string from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixel should display 
 */
void WS2812_framedata_setPixel(uint8_t row, uint16_t column, uint32_t rgb)
{
	uint8_t i;
    uint8_t red   = (uint8_t)((rgb & 0x00FF0000) >> 16);
    uint8_t green = (uint8_t)((rgb & 0x0000FF00) >> 8);
    uint8_t blue  = (uint8_t)(rgb & 0x000000FF);

	for (i = 0; i < 8; i++)
	{
		// clear the data for pixel 
		WS2812_IO_framedata[((column * 24) + i)] &= ~(0x01 << row);
		WS2812_IO_framedata[((column * 24) + 8 + i)] &= ~(0x01 << row);
		WS2812_IO_framedata[((column * 24) + 16 + i)] &= ~(0x01 << row);
		// write new data for pixel
		WS2812_IO_framedata[((column * 24) + i)] |= ((((green << i) & 0x80) >> 7) << row);
		WS2812_IO_framedata[((column * 24) + 8 + i)] |= ((((red << i) & 0x80) >> 7) << row);
		WS2812_IO_framedata[((column * 24) + 16 + i)] |= ((((blue << i) & 0x80) >> 7) << row);
	}
}

/* This function is a wrapper function to set all LEDs in the complete row to the specified color
 * 
 * Arguments:
 * row = the channel number/LED strip to set the color of from 0 to 15
 * columns = the number of LEDs in the strip to set to the color from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixels should display 
 */
void WS2812_framedata_setRow(uint8_t row, uint16_t columns, uint32_t rgb)
{
	uint8_t i;
	for (i = 0; i < columns; i++)
	{
		WS2812_framedata_setPixel(row, i, rgb);
	}
}

/* This function is a wrapper function to set all the LEDs in the column to the specified color
 * 
 * Arguments:
 * rows = the number of channels/LED strips to set the row in from 0 to 15
 * column = the column/LED position in the LED string from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixels should display 
 */
void WS2812_framedata_setColumn(uint8_t rows, uint16_t column, uint32_t rgb)
{
	uint8_t i;
	for (i = 0; i < rows; i++)
	{
		WS2812_framedata_setPixel(i, column, rgb);
	}
}

uint32_t tone_correction_func(uint32_t input_tone, 
							   uint8_t r_index,
						 rgb_operation r_op,
							   uint8_t g_index,
						 rgb_operation g_op,
						       uint8_t b_index,
						 rgb_operation b_op)
{
	uint32_t out = 0x00000000;
	uint8_t red   = (uint8_t)((input_tone & 0x00FF0000) >> 16);
	uint8_t green = (uint8_t)((input_tone & 0x0000FF00) >> 8);
	uint8_t blue  = (uint8_t)(input_tone & 0x000000FF);
	
	switch (r_op)
	{
		case ADD:
		
		// проверяем на переполнение.
		if ((red + r_index) <= 0xFF)
		{
			red += r_index;			
		}
		else
		{
			red = 0xFF;
		}

	    break;
		
		case SUB:
		
		// проверяем на переполнение.
		if ((red - r_index) >= 0x00)
		{
			red -= r_index;			
		}
		else
		{
			red = 0x00;
		}
		
		break;		
	
	default:
		break;
	}
	
	switch (g_op)
	{
	case ADD:
		
	    // проверяем на переполнение.
		if ((green + g_index) <= 0xFF)
		{
			green += g_index;			
		}
		else
		{
			green = 0xFF;
		}

		break;
		
	case SUB:
		
	    // проверяем на переполнение.
		if ((green - g_index) >= 0x00)
		{
			green -= g_index;			
		}
		else
		{
			green = 0x00;
		}
		
		break;		
	
	default:
		break;
	}
	
	switch (b_op)
	{
	case ADD:
		
	    // проверяем на переполнение.
		if ((blue + b_index) <= 0xFF)
		{
			blue += b_index;			
		}
		else
		{
			blue = 0xFF;
		}

		break;
		
	case SUB:
		
	    // проверяем на переполнение.
		if ((blue - b_index) >= 0x00)
		{
			blue -= b_index;			
		}
		else
		{
			blue = 0x00;
		}
		
		break;		
	
	default:
		break;
	}
	
	out |= ((uint32_t)red << 16);
	out |= ((uint32_t)green << 8);
	out |=  (uint32_t)blue;
	
	return out;
}

int main(void) 
{	
	uint8_t i,j;
	uint32_t rgb_after_correct;
	uint32_t rng_val;
	
	GPIO_init();
	DMA_init();
	TIM2_init();		// таймер для работы с WS2812
	adc_rng_init();	    // АЦП для получения случайного числа.
	void USART_init();	// Настраиваем USART1 для работы с HC-06
	
	while (1) {
		// set two pixels (columns) in the defined row (channel 0) to the
		// color values defined in the colors array
	//	for (i = 0; i < NUM_OF_FRAMES; i++)
	//	{
						
			srand(adc_rng_get());   // зерно для получения случайного числа.
			
		    i = rand() % 24;	    // случайное число.
			
			for (j = 0; j < NUMOFLEDS; j++)
			{
				// wait until the last frame was transmitted
				while (!WS2812_TC);
			
				// корректируем оттенок всего костра.
				rgb_after_correct = tone_correction_func(frames[i][j], 0xFF, ADD, 0x57, SUB, 0x37, SUB);
				
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




