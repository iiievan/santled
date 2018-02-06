#ifndef __LOW_LEVEL_INIT_H
#define __LOW_LEVEL_INIT_H

#include <stm32f10x_conf.h>
#include <stm32f10x.h>
#include <ws2812_lib.h>

#define WS2812_IO_High  0xFFFF;
#define WS2812_IO_Low   0x0000;

void GPIO_init(void);
void TIM2_init(void);
void TIM3_init(void);
void DMA_init(void);
void USART_init(void);

#endif