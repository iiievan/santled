#include <stm32f10x_conf.h>
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

#include <stm32f10x.h>

/* this define sets the number of TIM2 overflows
 * to append to the data frame for the LEDs to 
 * load the received data into their registers */
#define WS2812_DEADPERIOD 19
#define NUMOFLEDS 29
#define BUFFERSIZE (NUMOFLEDS*24)
#define NUM_OF_FRAMES 7
#define RGB 3

//union
//{
//    uint32_t RGB;
//    struct
//    {
//        uint8_t R;
//        uint8_t G;
//        uint8_t B;
//        uint8_t reserve;
//    } ch;
//} color_t;
//
//color_t pixel = 0x00AABBCC;
//
//pixel.RGB = 0x00AABBCC;
//
//pixel.ch.R = FF;
//
//color_t frames[][] = 
// попробовать потом вот это.

uint16_t WS2812_IO_High = 0xFFFF;
uint16_t WS2812_IO_Low = 0x0000;

volatile uint8_t WS2812_TC = 1;
volatile uint8_t TIM2_overflows = 0;

/* WS2812 framebuffer
 * buffersize = (#LEDs / 16) * 24 */
uint16_t WS2812_IO_framedata[BUFFERSIZE];

/* Array defining 12 color triplets to be displayed */
uint8_t colors[12][3] = 
{
	{ 0xFF, 0x00, 0x00 },
	{ 0xFF, 0x80, 0x00 },
	{ 0xFF, 0xFF, 0x00 },
	{ 0x80, 0xFF, 0x00 },
	{ 0x00, 0xFF, 0x00 },
	{ 0x00, 0xFF, 0x80 },
	{ 0x00, 0xFF, 0xFF },
	{ 0x00, 0x80, 0xFF },
	{ 0x00, 0x00, 0xFF },
	{ 0x80, 0x00, 0xFF },
	{ 0xFF, 0x00, 0xFF },
	{ 0xFF, 0x00, 0x80 }
};

//  {
//      0X00, 0X00 ,0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
//      0X00, 0X00 ,0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 
//      0X00, 0X00 ,0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 
//      0X00, 0X00 ,0X00, 0X00, 0X00, 0X00000000, 0X00000000, 0X00000000 
//  }, 
static uint32_t frames[NUM_OF_FRAMES][32] = 
{
	//      1	        2		    3			4			5			6		    7		    8      
	//      9	        10		    11  		12			13			14		    15          16  
	//      17	        18		    19			20			21			22		    23		    24  
	//      25	        26		    27			28			29			30		    31		    32  
	// 1-й кадр
	{   
        0x00F1EBBB, 0x00F1EBBB, 0x00F1EBBB, 0x00EFEBA2, 0x00EFEB88, 0x00EEE46B, 0x00C9AF27, 0x00A87C00,
        0X00AF6B00, 0X00EE8438, 0X00C42B01, 0X00CA1303, 0X00BD0000, 0X00B50000, 0X009F0000, 0X00920300,
        0X00860000, 0X00850900, 0X00710101, 0X006A0608, 0X00590302, 0X00560806, 0X004A0300, 0X00480701, 
        0X00400604, 0X00390706, 0X002F0705, 0X001D0C04, 0X001D0B06, 0X00000000, 0X00000000, 0X00000000   
	},
	
	// 2-й кадр
	{
		0X00F0CAB6, 0X00F0CAB6 ,0X00F0CAB6, 0X00ECC89C, 0X00ECC888, 0X00F2C670, 0X00F9C052, 0X00FFE24F,   
		0X00C38705, 0X009F4400 ,0X00A72800, 0X00B21D00, 0X00AF1200, 0X00A90E00, 0X009D0C00, 0X00950B00,
		0X00930D02, 0X00890C06 ,0X00730301, 0X006B0707, 0X005C0402, 0X00580904, 0X004B0400, 0X00470600,                                         
		0X00400604, 0X00380806 ,0X002E0805, 0X00260905, 0X001F0604, 0X00000000, 0X00000000, 0X00000000 
	},                                    
   
	// 3-й кадр
	{
		0X00F4F3BB, 0X00F4F3BB ,0X00F4F3BB, 0X00F4ECA3, 0X00F1DD87, 0X00FFE574, 0X00F1D43A, 0X00D3B20D,   
		0X00CFA616, 0X00D19C1A ,0X00CE8A0D, 0X00DA881A, 0X00B75500, 0X00A53E00, 0X00841C00, 0X00801200, 
		0X00870A00, 0X007C0000 ,0X00810A0C, 0X006C0107, 0X00620307, 0X00540000, 0X00500700, 0X00400100,                                          
		0X00350200, 0X00370D0F ,0X0025060B, 0X0021080B, 0X001F0B0A, 0X00000000, 0X00000000, 0X00000000 
	},
	
	// 4-й кадр
	{
    	0X00EBEAB1, 0X00EBEAB1 ,0X00EBEAB1, 0X00F3EBA2, 0X00F7E58F, 0X00F2D762, 0X00FFE03A, 0X00F0CC14,
    	0X00FFDC2E, 0X00FFDB32 ,0X00FCD027, 0X00FFD032, 0X00FFD44D, 0X00FFC859, 0X00F9953D, 0X009C2900, 
    	0X008F0F00, 0X00880400 ,0X00780000, 0X00780502, 0X00650000, 0X00630A06, 0X004E0100, 0X004E0E05, 
    	0X003D0705, 0X00390D0E ,0X0029060A, 0X0024070B, 0X00220A0A, 0X00000000, 0X00000000, 0X00000000 
    }, 
	
	// 5-й кадр
	{
    	0X00ECE8AB, 0X00ECE8AB ,0X00ECE8AB, 0X00F0E69F, 0X00FFEE9E, 0X00F3DD6E, 0X00FEE445, 0X00F9D71E,
    	0X00FED219, 0X00FFD91A ,0X00FAD70B, 0X00F0CF06, 0X00FFDA23, 0X00E6AE0F, 0X00FFAE2C, 0X00FFA13A, 
    	0X00CE5D0D, 0X00A63200 ,0X00800B00, 0X00780900, 0X005C0000, 0X005F0700, 0X004C0200, 0X00440500, 
    	0X003D0705, 0X00370B0C ,0X002D0609, 0X00260607, 0X00250906, 0X00000000, 0X00000000, 0X00000000 
    },

    // 6-й кадр
    {
    	0X00F3EBAD, 0X00F3EBAD ,0X00F3EBAD, 0X00FAEBA6, 0X00F4E594, 0X00F0DF73, 0X00F6E347, 0X00F3D623,
    	0X00FFD520, 0X00FBCC0D ,0X00FFE313, 0X00FEDF0B, 0X00FDDB0F, 0X00F3C70C, 0X00E39F00, 0X00D88200, 
    	0X00DD7F0D, 0X00CB6910 ,0X00BD561B, 0X008D2400, 0X006A0A00, 0X00560000, 0X00550C00, 0X004B0D02, 
    	0X003D0603, 0X00370707 ,0X00330708, 0X002D0706, 0X00260702, 0X00000000, 0X00000000, 0X00000000 
    },

    // 7-й кадр
    {
    	0X00EFE2AB, 0X00EFE2AB ,0X00EFE2AB, 0X00F2E19B, 0X00FFF39D, 0X00E8DE6D, 0X00E9E047, 0X00F4DD35,
    	0X00E6B918, 0X00E4AA08 ,0X00E3AC00, 0X00EAB900, 0X00FFE723, 0X00FAD20C, 0X00FFD117, 0X00FFC118, 
    	0X00EBA512, 0X00D68913 ,0X00F09A47, 0X00D97E47, 0X00AD502E, 0X00630C00, 0X00520600, 0X00450200, 
    	0X00410602, 0X00390303 ,0X00370506, 0X00310502, 0X00280600, 0X00000000, 0X00000000, 0X00000000
    },  

    // 8-й кадр
    {
    	0X00F7F9BA, 0X00F7F9BA ,0X00F7F9BA, 0X00E9EE9E, 0X00EDED93, 0X00F3E97A, 0X00FFE65A, 0X00B58F00,
    	0X00A8840C, 0X00BD961D ,0X00E2B824, 0X00D2A702, 0X00D3A900, 0X00FFDA35, 0X00F3C532, 0X00FFCF4A, 
    	0X00FFBF41, 0X00FFBB4F ,0X00C2762A, 0X006A1800, 0X00771400, 0X00851F1B, 0X005B0101, 0X004E0307, 
    	0X003D0508, 0X00360509 ,0X003E090F, 0X00340306, 0X00280702, 0X00000000, 0X00000000, 0X00000000 
    }, 

    //9-й кадр
    {
    	0X00E9EBAC, 0X00E9EBAC ,0X00E9EBAC, 0X00EAF098, 0X00E9EC81, 0X00EFE665, 0X00FFE64D, 0X00D2AA0D,
    	0X00B38A00, 0X008F6300 ,0X007F4E00, 0X00A77000, 0X00C89200, 0X00D59C07, 0X00DD9D15, 0X00F9AB2D, 
    	0X00F89722, 0X00E37514 ,0X00A53400, 0X008A1400, 0X00800500, 0X00720000, 0X00660000, 0X00570000, 
    	0X00450000, 0X00400003 ,0X003E0005, 0X00360204, 0X00290600, 0X00000000, 0X00000000, 0X00000000 
    }, 

    // 10-й кадр
    {
    	0X00EDEFAD, 0X00EDEFAD ,0X00EDEFAD, 0X00EEF598, 0X00ECF17B, 0X00E9E256, 0X00FFE747, 0X00FFD62E,
    	0X00FFD32F, 0X00F3C529 ,0X00F3BA2D, 0X00E7A721, 0X00FBB931, 0X00FBB22F, 0X00EB981E, 0X00E38313, 
    	0X00E5710A, 0X00E2610E ,0X00E25923, 0X00D64A29, 0X00C43726, 0X009E1910, 0X00830E04, 0X006E0700, 
    	0X005C0107, 0X00510000 ,0X00470000, 0X003D0101, 0X00300904, 0X00000000, 0X00000000, 0X00000000
    }  
};  

/* simple delay counter to waste time, don't rely on for accurate timing */
void Delay(__IO uint32_t nCount) {
	while (nCount--) {
	}
}

void GPIO_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	// GPIOA Periph clock enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// GPIOA pins WS2812 data outputs
	GPIO_InitStructure.GPIO_Pin = 0x00FF;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
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

int main(void) 
{	
	uint8_t i,j;
	
	GPIO_init();
	DMA_init();
	TIM2_init();
	
	while (1) {
		// set two pixels (columns) in the defined row (channel 0) to the
		// color values defined in the colors array
		for (i = 0; i < NUM_OF_FRAMES; i++)
		{
			for (j = 0; j < NUMOFLEDS; j++)
			{
				// wait until the last frame was transmitted
				while (!WS2812_TC);
			
				// this approach sets each pixel individually
				WS2812_framedata_setPixel(4, j, frames[i][j]);
				WS2812_framedata_setPixel(5, j, frames[i][j]);
				WS2812_framedata_setPixel(6, j, frames[i][j]);
				WS2812_framedata_setPixel(7, j, frames[i][j]);
		
			
				// this funtion is a wrapper and achieved the same thing, tidies up the code
				//	WS2812_framedata_setRow(4, 33, colors[i][0], colors[i][1], colors[i][2]);
				//	WS2812_framedata_setRow(5, 33, colors[i][0], colors[i][1], colors[i][2]);
				//	WS2812_framedata_setRow(6, 33, colors[i][0], colors[i][1], colors[i][2]);
				//	WS2812_framedata_setRow(7, 33, colors[i][0], colors[i][1], colors[i][2]);
				//	WS2812_framedata_setRow(4, LED_NUM - 1, frames[i][0], frames[i][1], frames[i][2]);
				//	WS2812_framedata_setRow(5, LED_NUM, frames[i][0], frames[i][1], frames[i][2]);
				//	WS2812_framedata_setRow(6, LED_NUM, frames[i][0], frames[i][1], frames[i][2]);
				//	WS2812_framedata_setRow(7, LED_NUM, frames[i][0], frames[i][1], frames[i][2]);
					// send the framebuffer out to the LEDs
				//	WS2812_sendbuf(BUFFERSIZE);
					// wait some amount of time
				//	Delay(1000000);
			}
		
			WS2812_sendbuf(BUFFERSIZE);
			Delay(111111);			
		}				
	}
}




