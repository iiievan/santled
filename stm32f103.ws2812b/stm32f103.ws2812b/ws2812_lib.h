#ifndef _WS2812_LIB_H
#define _WS2812_LIB_H

#include <stm32f10x_conf.h>

#define NUMOFLEDS 29
#define BUFFERSIZE (NUMOFLEDS*24)

extern uint8_t WS2812_TC;

/* буффер кадров WS2812 
 * размер буффера = (#LEDs / 16) * 24 */
uint16_t WS2812_IO_framedata[BUFFERSIZE];

typedef enum { ADD, SUB } rgb_operation;	// тип операции для функции коррекции тона tone_correction_func
typedef enum { RED, GREEN, BLUE }rgb_mask;  // цвет для который нужно выделить из переменной 0x00RRGGBB.

void WS2812_sendbuf(uint32_t);
void WS2812_framedata_setPixel(uint8_t, uint16_t, uint32_t);
void WS2812_framedata_setRow(uint8_t, uint16_t, uint32_t);
void WS2812_framedata_setColumn(uint8_t, uint16_t, uint32_t);
uint32_t tone_correction_func(uint32_t, uint8_t, rgb_operation, uint8_t, rgb_operation, uint8_t, rgb_operation);
uint8_t put_rgb_mask(uint32_t, rgb_mask);
rgb_operation eject_operation(uint32_t, rgb_mask);

#endif
