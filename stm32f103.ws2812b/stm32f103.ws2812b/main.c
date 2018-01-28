
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

uint32_t usart_recieve = 0;				// получаемые данные из USART
static uint8_t usart_rx_byte_conter = 0;	// счетчик байт принятого сообщения.
static bool you_have_new_message = false;
static bool usart_rxtx = false;				// идет прием/передача по usart
static uint16_t usart_buffer_reset_tmr = 0;

static uint8_t    red_coeff   = 0xFF,
		          green_coeff = 0x03,
		          blue_coeff  = 0x03;

ring_buffer usart_buffer = { 0 };

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
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM3_init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    uint16_t PrescalerValue;
    
    // TIM3 Periph clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    PrescalerValue = (uint16_t)(SystemCoreClock / 72000000) - 1;
    // конфигурируем таймер 3 для подсчета интервала между принимаемыми сообщениями по USART
	TIM_TimeBaseStructure.TIM_Period = 10; 
    TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

   /* Разрешаем прерывание по обновлению (в данном случае -
   * по переполнению) счётчика таймера TIM3.
   */
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
		/* Выключаем таймер */
	TIM_Cmd(TIM3, DISABLE);

    
    /* configure TIM3 interrupt */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* enable DMA1 Channel7 transfer complete interrupt */
	DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);
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
		
		/* Включаем USART1 и прерывания от него для приема нового цвета */
		USART_Cmd(USART1, ENABLE);
	}
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		
		usart_buffer_reset_tmr++;
		
		if (usart_buffer_reset_tmr >= UART_BUFER_RESET_INTERVAL)
		{
			usart_rxtx = false;
			// указатель в начало буфера (обнуляем буфер для приема новых данных)
			usart_buffer.head = usart_buffer.tail = 0;			
			// передача завершена, извещаем об этом всех и останавливаем таймер.
			TIM_Cmd(TIM3, DISABLE);
		}
	}
}

/*
 *  Функция обработчик прерывания USARTx.
 */
void USART1_IRQHandler(void)
{
	uint8_t rx_temp = 0xFF,
			bufer_status = RB_OK;    
	
	usart_sync_read(&rx_temp);	// получаем байт данных
	
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	
	// если приняли заголовок, тогда считаем байты - всего восемь.
	// общий формат сообщения 0A BC 40 FF 47 47 00 - для начального кадра (костер)
	// где 0А BC - это заголовок для смены цвета эффекта.
	// 0x40 - код операции, 0RGB 0000 - там где единица, там операция  сложить, где ноль вычесть.

	if ((rx_temp == 0x0A && 
		 usart_buffer.head == 0) ||		// первый байт заголовка сообщения о корректировке
		(usart_buffer.head == 1 &&	    // второй байт заголовка сообщения о корректировке
		 rx_temp == 0xBC) ||
		usart_buffer.head > 1 )
	{
			
		bufer_status = rb_write(&usart_buffer, &rx_temp, 1);	// пишем байт в кольцевой буффер	

		if (bufer_status != RB_OK)
		{			
			// выделяем цвета из коэффициента что пришел по Bluetooth
			// и сохраняем их.
			red_coeff   = usart_buffer.storage[3];	
			green_coeff = usart_buffer.storage[4];	
			blue_coeff  = usart_buffer.storage[5];	
			
			you_have_new_message = true;	// флаг для обновления цвета в лентах.
			
			usart_buffer.head = usart_buffer.tail = 0;	// обнуляем буффер для приема следующего кадра.
			
			// передача завершена, извещаем об этом всех и останавливаем таймер.
			TIM_Cmd(TIM3, DISABLE);	// Запускаем измерение интервала времени между посылками.

			usart_rxtx = false;
		}
		else
		{
			if (usart_rxtx != true)
			{
				TIM_Cmd(TIM3, ENABLE);	// Запускаем измерение интервала времени между посылками.
				usart_buffer_reset_tmr = 0;
				usart_rxtx = true;	    //  идет прием пакета по usart.	
			}
			
			usart_buffer_reset_tmr = 0;
		}
		

	}	
}

int main(void) 
{	
	static uint8_t i,j;
	static uint32_t input_rgb_tone;	// переменная содержащая цвета пикселей после коррекции
	// задаем начальные кадры для костра.
	static rgb_operation red_op   = ADD,
						 green_op = SUB,
						 blue_op  = SUB;
	uint16_t buffer_val = 0;

	static frame_t rgb_frame;
	rgb_frame.pixel = (uint8_t)NUMOFLEDS;	// инициализаця
	
	GPIO_init();
	DMA_init();
	TIM2_init();			// таймер для работы с WS2812
	TIM3_init();			// таймер для измерения промежутков врмени.
	adc_rng_init();		    // АЦП для получения случайного числа.
	
	rb_init(&usart_buffer);	// инициализируем буффер только перед инициализацией usart,
							// там разрешается прерывание в конце. 
	usart_init();	        // настраиваем USART1 для работы с HC-06
	
	
	while (1) 
	{	
		// если пришло новое сообщение по USART то обновляем цвет.
		if (you_have_new_message == true)
		{
			srand(adc_rng_get());   // зерно для получения случайного числа.
		
			i = rand() % 10;	    // случайный  кадр из 24-х.
					
			input_rgb_tone |= ((uint32_t)red_coeff << 16);
			input_rgb_tone |= ((uint32_t)green_coeff << 8);
			input_rgb_tone |=  (uint32_t)blue_coeff;
		
			for (j = 0; j < NUMOFLEDS; j++)
			{
			    // ждем пока не передастся последний кадр.
			    while (!WS2812_TC);		
			
				WS2812_framedata_setPixel(4, j, input_rgb_tone);
				WS2812_framedata_setPixel(5, j, input_rgb_tone);
				WS2812_framedata_setPixel(6, j, input_rgb_tone);
				WS2812_framedata_setPixel(7, j, input_rgb_tone);
			}
		
			if (usart_rxtx != true)
			{
				WS2812_sendbuf(BUFFERSIZE);
			}

			you_have_new_message = false;
		}

		//Delay(400000);						
	}
}




