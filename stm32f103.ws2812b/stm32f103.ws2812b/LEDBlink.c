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
#define NUM_OF_FRAMES 14
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
 
static uint32_t frames[NUM_OF_FRAMES][32] = 
{ 
	// 1-й кадр
	{   
        0x00f1ebbb, 0x00f1ebbb, 0x00f1ebbb, 0x00efeba2, 0x00efeb88, 0x00eee46b, 0x00c9af27, 0x00a87c00,
        0x00af6b00, 0x00ee8438, 0x00c42b01, 0x00ca1303, 0x00bd0000, 0x00b50000, 0x009f0000, 0x00920300,
        0x00860000, 0x00850900, 0x00710101, 0x006a0608, 0x00590302, 0x00560806, 0x004a0300, 0x00480701, 
        0x00400604, 0x00390706, 0x002f0705, 0x001d0c04, 0x001d0b06, 0x00000000, 0x00000000, 0x00000000   
	},
	
	// 2-й кадр
	{
		0x00f0cab6, 0x00f0cab6 ,0x00f0cab6, 0x00ecc89c, 0x00ecc888, 0x00f2c670, 0x00f9c052, 0x00ffe24f,   
		0x00c38705, 0x009f4400 ,0x00a72800, 0x00b21d00, 0x00af1200, 0x00a90e00, 0x009d0c00, 0x00950b00,
		0x00930d02, 0x00890c06 ,0x00730301, 0x006b0707, 0x005c0402, 0x00580904, 0x004b0400, 0x00470600,                                         
		0x00400604, 0x00380806 ,0x002e0805, 0x00260905, 0x001f0604, 0x00000000, 0x00000000, 0x00000000 
	},                                    
   
	// 3-й кадр
	{
		0x00f4f3bb, 0x00f4f3bb ,0x00f4f3bb, 0x00f4eca3, 0x00f1dd87, 0x00ffe574, 0x00f1d43a, 0x00d3b20d,   
		0x00cfa616, 0x00d19c1a ,0x00ce8a0d, 0x00da881a, 0x00b75500, 0x00a53e00, 0x00841c00, 0x00801200, 
		0x00870a00, 0x007c0000 ,0x00810a0c, 0x006c0107, 0x00620307, 0x00540000, 0x00500700, 0x00400100,                                          
		0x00350200, 0x00370d0f ,0x0025060b, 0x0021080b, 0x001f0b0a, 0x00000000, 0x00000000, 0x00000000 
	},
	
	// 4-й кадр
	{
    	0x00ebeab1, 0x00ebeab1 ,0x00ebeab1, 0x00f3eba2, 0x00f7e58f, 0x00f2d762, 0x00ffe03a, 0x00f0cc14,
    	0x00ffdc2e, 0x00ffdb32 ,0x00fcd027, 0x00ffd032, 0x00ffd44d, 0x00ffc859, 0x00f9953d, 0x009c2900, 
    	0x008f0f00, 0x00880400 ,0x00780000, 0x00780502, 0x00650000, 0x00630a06, 0x004e0100, 0x004e0e05, 
    	0x003d0705, 0x00390d0e ,0x0029060a, 0x0024070b, 0x00220a0a, 0x00000000, 0x00000000, 0x00000000 
    }, 
	
	// 5-й кадр
	{
    	0x00ece8ab, 0x00ece8ab ,0x00ece8ab, 0x00f0e69f, 0x00ffee9e, 0x00f3dd6e, 0x00fee445, 0x00f9d71e,
    	0x00fed219, 0x00ffd91a ,0x00fad70b, 0x00f0cf06, 0x00ffda23, 0x00e6ae0f, 0x00ffae2c, 0x00ffa13a, 
    	0x00ce5d0d, 0x00a63200 ,0x00800b00, 0x00780900, 0x005c0000, 0x005f0700, 0x004c0200, 0x00440500, 
    	0x003d0705, 0x00370b0c ,0x002d0609, 0x00260607, 0x00250906, 0x00000000, 0x00000000, 0x00000000 
    },

    // 6-й кадр
    {
    	0x00f3ebad, 0x00f3ebad ,0x00f3ebad, 0x00faeba6, 0x00f4e594, 0x00f0df73, 0x00f6e347, 0x00f3d623,
    	0x00ffd520, 0x00fbcc0d ,0x00ffe313, 0x00fedf0b, 0x00fddb0f, 0x00f3c70c, 0x00e39f00, 0x00d88200, 
    	0x00dd7f0d, 0x00cb6910 ,0x00bd561b, 0x008d2400, 0x006a0a00, 0x00560000, 0x00550c00, 0x004b0d02, 
    	0x003d0603, 0x00370707 ,0x00330708, 0x002d0706, 0x00260702, 0x00000000, 0x00000000, 0x00000000 
    },

    // 7-й кадр
    {
    	0x00efe2ab, 0x00efe2ab ,0x00efe2ab, 0x00f2e19b, 0x00fff39d, 0x00e8de6d, 0x00e9e047, 0x00f4dd35,
    	0x00e6b918, 0x00e4aa08 ,0x00e3ac00, 0x00eab900, 0x00ffe723, 0x00fad20c, 0x00ffd117, 0x00ffc118, 
    	0x00eba512, 0x00d68913 ,0x00f09a47, 0x00d97e47, 0x00ad502e, 0x00630c00, 0x00520600, 0x00450200, 
    	0x00410602, 0x00390303 ,0x00370506, 0x00310502, 0x00280600, 0x00000000, 0x00000000, 0x00000000
    },  

    // 8-й кадр
    {
    	0x00f7f9ba, 0x00f7f9ba ,0x00f7f9ba, 0x00e9ee9e, 0x00eded93, 0x00f3e97a, 0x00ffe65a, 0x00b58f00,
    	0x00a8840c, 0x00bd961d ,0x00e2b824, 0x00d2a702, 0x00d3a900, 0x00ffda35, 0x00f3c532, 0x00ffcf4a, 
    	0x00ffbf41, 0x00ffbb4f ,0x00c2762a, 0x006a1800, 0x00771400, 0x00851f1b, 0x005b0101, 0x004e0307, 
    	0x003d0508, 0x00360509 ,0x003e090f, 0x00340306, 0x00280702, 0x00000000, 0x00000000, 0x00000000 
    }, 

    //9-й кадр
    {
    	0x00e9ebac, 0x00e9ebac ,0x00e9ebac, 0x00eaf098, 0x00e9ec81, 0x00efe665, 0x00ffe64d, 0x00d2aa0d,
    	0x00b38a00, 0x008f6300 ,0x007f4e00, 0x00a77000, 0x00c89200, 0x00d59c07, 0x00dd9d15, 0x00f9ab2d, 
    	0x00f89722, 0x00e37514 ,0x00a53400, 0x008a1400, 0x00800500, 0x00720000, 0x00660000, 0x00570000, 
    	0x00450000, 0x00400003 ,0x003e0005, 0x00360204, 0x00290600, 0x00000000, 0x00000000, 0x00000000 
    }, 

    // 10-й кадр
    {
    	0x00edefad, 0x00edefad ,0x00edefad, 0x00eef598, 0x00ecf17b, 0x00e9e256, 0x00ffe747, 0x00ffd62e,
    	0x00ffd32f, 0x00f3c529 ,0x00f3ba2d, 0x00e7a721, 0x00fbb931, 0x00fbb22f, 0x00eb981e, 0x00e38313, 
    	0x00e5710a, 0x00e2610e ,0x00e25923, 0x00d64a29, 0x00c43726, 0x009e1910, 0x00830e04, 0x006e0700, 
    	0x005c0107, 0x00510000 ,0x00470000, 0x003d0101, 0x00300904, 0x00000000, 0x00000000, 0x00000000
    }, 

    // 11-й кадр
    {
        0x00f1f3aa, 0x00f1f3aa ,0x00f1f3aa, 0x00eeef92, 0x00eaea7a, 0x00ece45f, 0x00eed53b, 0x00fad637,
        0x00f7d23b, 0x00ffd143 ,0x00ffcd4b, 0x00ffbd3e, 0x00efa925, 0x00d88b15, 0x00f79c49, 0x00bc530f, 
        0x00e67227, 0x00e06529 ,0x00d55e40, 0x00d05546, 0x00cd4a36, 0x00bf3c28, 0x00bb402e, 0x00b24234, 
        0x0088251f, 0x008b332f ,0x0076281e, 0x005a1c11, 0x00380b08, 0x00000000, 0x00000000, 0x00000000 
    },

  	// 12-й кадр
    {
        0x00f0f0a6, 0x00f0f0a6, 0x00f0f0a6, 0x00e6e684, 0x00eef170, 0x00f1ec54, 0x00fce43a, 0x00fbd727,
        0x00efc91c, 0x00fccc2c ,0x00f7b72f, 0x00ce8407, 0x00ba6c00, 0x00b55f00, 0x00cb6415, 0x00af3b00, 
        0x00a52a00, 0x00bf3e14 ,0x00c43c2c, 0x00d54c44, 0x00d34e3f, 0x00c1422f, 0x00ad3622, 0x00942615, 
        0x00821b12, 0x00731611 ,0x00520300, 0x003e0100, 0x00340810, 0x00000000, 0x00000000, 0x00000000 
    },

	// 13-й кадр
    {
        0x00f7f0a8, 0x00f7f0a8 ,0x00f7f0a8, 0x00d6d46f, 0x00cfd245, 0x00dcd731, 0x00f6e02b, 0x00ffdf25,
        0x00ffde25, 0x00eec015 ,0x00e4a617, 0x00eea525, 0x00efa027, 0x00ee922b, 0x00e3752a, 0x00c04712, 
        0x00a72d04, 0x00830500 ,0x00940b05, 0x00860000, 0x00770000, 0x00780900, 0x00720e00, 0x005d0000, 
        0x005a0000, 0x00530103 ,0x00420002, 0x00340106, 0x0029040c, 0x00000000, 0x00000000, 0x00000000 
    },

    // 14-й кадр
    {
        0x00f9f1a9, 0x00f9f1a9 ,0x00f9f1a9, 0x00e7e27c, 0x00cfd140, 0x00dfcf22, 0x00d9c30b, 0x00e5c407,
        0x00e1bb00, 0x00e4b608 ,0x00e4a613, 0x00eca624, 0x00e7981f, 0x00e98d28, 0x00f07e37, 0x00f87b4b, 
        0x00eb7253, 0x00c94e3e ,0x00860000, 0x008b090b, 0x006c0000, 0x005f0000, 0x00630c03, 0x00580801, 
        0x004c0000, 0x00480105 ,0x003c070d, 0x00300610, 0x0024030e, 0x00000000, 0x00000000, 0x00000000 
    },

    // 15-й кадр
    {
        0x00fceaa8, 0x00fceaa8 ,0x00fceaa8, 0x00f0e089, 0x00fdf179, 0x00e4d641, 0x00e0cc1f, 0x00e1ca1a,
        0x00cfb61e, 0x00e2bb30 ,0x00dfa118, 0x00c07300, 0x00d17b0a, 0x00d47b21, 0x00b45a1e, 0x00a84621, 
        0x00ad3f26, 0x00c24d3c ,0x00cf564e, 0x00730100, 0x00650604, 0x00540000, 0x00550504, 0x00510b09, 
        0x00420807, 0x00350709 ,0x00260309, 0x0022030b, 0x002c040f, 0x00000000, 0x00000000, 0x00000000 
    },

	// 16-й кадр
    {
        0x00fdeeab, 0x00fdeeab ,0x00fdeeab, 0x00f9e991, 0x00f3e568, 0x00f6e74c, 0x00f6e034, 0x00fadf2e,
        0x00fadd37, 0x00dab316 ,0x00dca210, 0x00f6ad2a, 0x00f4a32e, 0x00a14800, 0x00802200, 0x00892600, 
        0x008c2006, 0x007c0800 ,0x00810600, 0x00780502, 0x005c0100, 0x00631513, 0x004a0000, 0x00500a0a, 
        0x003f0102, 0x003d0d0d ,0x002a0a0d, 0x0027080d, 0x002e070c, 0x00000000, 0x00000000, 0x00000000 
    }

    //      1	        2		    3			4			5			6		    7		    8      
	//      9	        10		    11  		12			13			14		    15          16  
	//      17	        18		    19			20			21			22		    23		    24  
	//      25	        26		    27			28			29			30		    31		    32 
};  

	// x-й кадр
//  {
//      0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//      0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//      0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//      0x00, 0x00 ,0x00, 0x00, 0x00, 0x00000000, 0x00000000, 0x00000000 
//  },

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




