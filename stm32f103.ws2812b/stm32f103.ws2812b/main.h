#ifndef __MAIN_H
#define __MAIN_H

#include <stm32f10x_conf.h>
#include <stm32f10x.h>
#include <adc.h>		    // случайное число
#include <stdlib.h>		    // srand, rand
#include <lightning_prg.h>	// тут храним световые алгоритмы и программы.

/* this define sets the number of TIM2 overflows
 * to append to the data frame for the LEDs to 
 * load the received data into their registers */
#define WS2812_DEADPERIOD 19
#define NUMOFLEDS 29
#define BUFFERSIZE (NUMOFLEDS*24)

typedef enum { ADD, SUB } rgb_operation;	// тип операции для функции коррекции тона tone_correction_func

/* буффер кадров WS2812 
 * размер буффера = (#LEDs / 16) * 24 */
uint16_t WS2812_IO_framedata[BUFFERSIZE];


void Delay(__IO uint32_t);
void GPIO_init(void);
void TIM2_init(void);
void DMA_init(void);
void USART_init(void);

void WS2812_sendbuf(uint32_t);
void DMA1_Channel7_IRQHandler(void);
void TIM2_IRQHandler(void);
void USART1_IRQHandler(void);
void WS2812_framedata_setPixel(uint8_t, uint16_t, uint32_t);
void WS2812_framedata_setRow(uint8_t, uint16_t, uint32_t);
void WS2812_framedata_setColumn(uint8_t, uint16_t, uint32_t);
uint32_t tone_correction_func(uint32_t, uint8_t, rgb_operation, uint8_t, rgb_operation, uint8_t, rgb_operation);
void UARTSend(const char *pucBuffer, uint32_t );


#endif // !_MAIN_H

