#ifndef __MAIN_H
#define __MAIN_H

#include <stm32f10x_conf.h>
#include <stm32f10x.h>
#include <adc.h>		    // случайное число
#include <stdlib.h>		    // srand, rand
#include <lightning_prg.h>	// тут храним световые алгоритмы и программы.
#include <ws2812_lib.h>
#include <ring_buffer.h>
#include <usart.h>
#include <ring_buffer.h>

#define __USE_C99_MATH	// для того чтобы тип bool был определен.
#include <stdbool.h>

/* this define sets the number of TIM2 overflows
 * to append to the data frame for the LEDs to 
 * load the received data into their registers */
#define WS2812_DEADPERIOD 19
#define USART_BUFFER_RESET_TIME 6

void Delay(__IO uint32_t);
void GPIO_init(void);
void TIM2_init(void);
void DMA_init(void);
void USART_init(void);


void DMA1_Channel7_IRQHandler(void);
void TIM2_IRQHandler(void);
void USART1_IRQHandler(void);
void UARTSend(const char *pucBuffer, uint32_t );


#endif // !_MAIN_H

