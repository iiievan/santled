#ifndef _WS2812_LIB_H
#define _WS2812_LIB_H

#include <stm32f10x_conf.h>
#include <stdbool.h>

#define NUMOFLEDS 29
#define BUFFERSIZE (NUMOFLEDS*24)

#define NUM_OF_FRAMES 24

extern bool ws2812_transmit;

/* буффер кадров WS2812 
 * размер буффера = (#LEDs / 16) * 24 */
uint16_t WS2812_IO_framedata[BUFFERSIZE];

typedef enum { ADD, SUB } rgb_operation;	// тип операции дл€ функции коррекции тона tone_correction_func
typedef enum { RED, GREEN, BLUE }rgb_mask;  // цвет дл€ который нужно выделить из переменной 0x00RRGGBB.

// представление пиксел€ RGB (red, green, blue)
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

// представление пиксел€ RGB (red, green, blue)
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

// заранее определенны значени€ цветого тона HSV формата (hue)
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

struct CRGB leds_buf[NUMOFLEDS];	// буфер дл€ бегущих эффектов.

void Delay(__IO uint32_t);	// __IO - volatile defined
void WS2812_sendbuf(uint32_t);
void WS2812_framedata_setPixel(uint8_t, uint16_t, uint32_t);
void WS2812_framedata_setRow(uint8_t, uint16_t, uint32_t);
void WS2812_framedata_setColumn(uint8_t, uint16_t, uint32_t);
uint8_t put_rgb_mask(uint32_t, rgb_mask);
rgb_operation eject_operation(uint32_t, rgb_mask);
uint32_t tone_correction_func(uint32_t, uint8_t, rgb_operation, uint8_t, rgb_operation, uint8_t, rgb_operation);

// Ёффекты.
void convert_rgb_to_dma_buf(struct CRGB *);
void hsv2rgb(struct CHSV *, struct CRGB *);
void move_leds(uint32_t, uint32_t, uint32_t, struct CRGB  *);
void fill_rainbow(const struct CHSV * , struct CRGB *);
void running_rainbow(struct CRGB  *);
void rotating_rainbow(struct CRGB  *);


#endif
