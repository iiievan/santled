#ifndef _WS2812_LIB_H
#define _WS2812_LIB_H

#include <stm32f10x_conf.h>
#include <stdbool.h>

#define NUMOFLEDS 29
#define BUFFERSIZE (NUMOFLEDS*24)

#define NUM_OF_FRAMES 24

extern bool ws2812_transmit;

/* ������ ������ WS2812 
 * ������ ������� = (#LEDs / 16) * 24 */
uint16_t WS2812_IO_framedata[BUFFERSIZE];

uint32_t leds_buf[NUMOFLEDS];	// ����� ��� ������� ��������.

typedef enum { ADD, SUB } rgb_operation;	// ��� �������� ��� ������� ��������� ���� tone_correction_func
typedef enum { RED, GREEN, BLUE }rgb_mask;  // ���� ��� ������� ����� �������� �� ���������� 0x00RRGGBB.

// ��������� ����� ��� ��� ������������.
typedef struct
{
	uint32_t pixels[NUMOFLEDS];	// ���������� ����������� � �����, �������� �� ���� ������� �� ������������.	
	uint8_t  pixel;				// ��������� �� ���������� ������� � �����.
	
} frame_t;

void Delay(__IO uint32_t);	// __IO - volatile defined
void WS2812_sendbuf(uint32_t);
void WS2812_framedata_setPixel(uint8_t, uint16_t, uint32_t);
void WS2812_framedata_setRow(uint8_t, uint16_t, uint32_t);
void WS2812_framedata_setColumn(uint8_t, uint16_t, uint32_t);
uint8_t put_rgb_mask(uint32_t, rgb_mask);
rgb_operation eject_operation(uint32_t, rgb_mask);
uint32_t tone_correction_func(uint32_t, uint8_t, rgb_operation, uint8_t, rgb_operation, uint8_t, rgb_operation);
void fill_frame(frame_t * const , uint32_t , uint8_t );

// �������.
void convert_rgb_to_dma_buf(uint32_t *);
uint32_t hsv_to_rgb(int, int, int);
uint32_t hsv_to_rgb_double(uint32_t, uint32_t, uint32_t);
void move_leds(uint32_t, uint32_t, uint32_t, uint32_t *);
void fill_rainbow(uint32_t, uint32_t, uint8_t, uint32_t *);
void running_rainbow(uint32_t *);


#endif
