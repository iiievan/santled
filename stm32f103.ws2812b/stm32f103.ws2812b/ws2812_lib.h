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

// ������������� ������� RGB (red, green, blue)
struct CHSV {
	union {
		struct {			
			union {
				uint8_t value;
				uint8_t val;
				uint8_t v;
			};
			union {
				uint8_t saturation;
				uint8_t sat;
				uint8_t s;
			};
			union {
				uint8_t hue;
				uint8_t h;
			};
		};
		uint32_t hsv;
	};
};

// ������������� ������� RGB (red, green, blue)
struct CRGB {
	union {
		struct {
			union {
				uint8_t b;
				uint8_t blue;
			};
			union {
				uint8_t g;
				uint8_t green;
			};
			union {
				uint8_t r;
				uint8_t red;
			};
		};
		
		uint32_t rgb;
	};
};

// ������� ����������� �������� ������� ���� HSV ������� (hue)
typedef enum {
	HUE_RED    = 0,
	HUE_ORANGE = 32,
	HUE_YELLOW = 64,
	HUE_GREEN  = 96,
	HUE_AQUA   = 128,
	HUE_BLUE   = 160,
	HUE_PURPLE = 192,
	HUE_PINK   = 224
} HSVHue;

void Delay(__IO uint32_t);	// __IO - volatile defined
void WS2812_sendbuf(uint32_t);
void WS2812_framedata_setPixel(uint8_t, uint16_t, uint32_t);
void WS2812_framedata_setRow(uint8_t, uint16_t, uint32_t);
void WS2812_framedata_setColumn(uint8_t, uint16_t, uint32_t);
uint8_t put_rgb_mask(uint32_t, rgb_mask);
rgb_operation eject_operation(uint32_t, rgb_mask);
uint32_t tone_correction_func(uint32_t, uint8_t, rgb_operation, uint8_t, rgb_operation, uint8_t, rgb_operation);

// �������.
void convert_rgb_to_dma_buf(uint32_t *);
uint32_t hsv_to_rgb(int, int, int);
uint32_t hsv_to_rgb_double(uint32_t, uint32_t, uint32_t);
void move_leds(uint32_t, uint32_t, uint32_t, uint32_t *);
void fill_rainbow(uint32_t, uint32_t, uint8_t, uint32_t *);
void running_rainbow(uint32_t *);
void rotating_rainbow(uint32_t *);


#endif
