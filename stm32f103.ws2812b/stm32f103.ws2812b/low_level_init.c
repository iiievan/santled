#include <low_level_init.h>

void low_level_init(void)
{
	GPIO_init();
	DMA_init();
	TIM2_init();			// ������ ��� ������ � WS2812	
	// �����������, ���� �� �������� ������, ��� ������ ������ ���������� TIM3_IRQHandler 
	// ����� �� ���������� ����� �� ���� � ������� LED
  //TIM3_init();			// ������ ��� ��������� ����������� ������ ����� ������� ������ �� USART
	adc_rng_init();		    // ��� ��� ��������� ���������� �����.	
 
	usart_init();	        // ����������� USART1 ��� ������ � HC-06
}

void adc_rng_init(void)
{
	ADC_InitTypeDef ADC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_InitStructure.ADC_NbrOfChannel          = ADC_Channel_12;
	ADC_InitStructure.ADC_Mode                  = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode          = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode    = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv      = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign             = ADC_DataAlign_Right;
	ADC_Init(ADC1, &ADC_InitStructure);


	//  ��� ����� �� ���������� ����� Vrefint - ��� 17-� �����.
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 1, ADC_SampleTime_7Cycles5);

	ADC_Cmd(ADC1, ENABLE);
}

void GPIO_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
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
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void TIM2_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	uint16_t PrescalerValue;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	PrescalerValue = (uint16_t)(SystemCoreClock / 72000000) - 1;
	/* ��������� �������� ������� */
	TIM_TimeBaseStructure.TIM_Period = 29; // ��������� �� 800kHz
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM2, DISABLE);

	/* �������� ������ ������� ������ 1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 8;
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);	

	/* �������� ������ ������� ������ 2 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 17;
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Disable);
	
	/* ��������� ���������� TIM2  */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  // �������� � ������(�������) ������ � ������ �����������.
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		   // TIM2 ������������ ������ DMA.
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	// ����������� � ������� ������ ����� ���������.
	//NVIC_SetPriority();
	//NVIC_EnableIRQ();
}

void TIM3_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
    
	uint16_t PrescalerValue;
    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	PrescalerValue = (uint16_t)(SystemCoreClock / 1000) - 1;	// ����� (72000 - 1)
	// ������������� ������ 3 ��� �������� ��������� ����� ������������ ����������� �� USART
	TIM_TimeBaseStructure.TIM_Period = 100;		// 100 must give 0.1 ms buffer reset interval, 1000 - 1sec
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM3, DISABLE);
	TIM_SetCounter(TIM3, 1);	// �� ���� ����� �������.

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	// �������� �� ������ ������ � ������ �����������.
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			// ������������ TIM3 ����� ���������� �� USART
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void DMA_init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	// TIM2 Update event
	/* ��������� DMA1 ����� 2 */
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
	/* ��������� DMA1 ����� 5 */
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
	/* ��������� DMA1 ����� 7 */
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


	// ��������� ��� ������ ����������� ����������,
	// DMA � TIM2 ����� ���������, ������� NVIC_IRQChannelPreemptionPriority = 0 - ��� ������(�������) ������
	// � ��� TIM2 ����� ������� ��������� ������ ��� ����� ������� �� ����� 50�� �������� ����� ������� �������� �
	// ws2812
	// USART1 � TIM3 �������� �� ������ ������, �.�. ���������� �� ��� ����� ����� � ������� � ������� ����������
	// �� BT.
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);			
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	// �������� � ������(�������) ������ � ������ �����������.
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			// DMA ������������ ����� TIM2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* enable DMA1 Channel7 transfer complete interrupt */
	DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);
}

void usart_init(void)
{
	const char welcome_str[] = " Welcome to Bluetooth!\r\n";
	
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* ����������� USART1 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* USART1 ��������������� ��� ���:
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
	
	USART_InitStructure.USART_BaudRate = 115200;		// �������� ��������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
	
    /* ����������� ���������� �� USART1 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	 // �������� �� ������ ������ � ������ �����������.
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			 // ������� ������������ USART, � ����� TIM3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

	/* �������� USART1 */
	USART_Cmd(USART1, ENABLE);
	/* �������� ���������� �� USART1 */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	/* ����������� � ��������. */
	uart_send(welcome_str, sizeof(welcome_str));
}