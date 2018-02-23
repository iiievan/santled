#ifndef __LIB_8_H
#define __LIB_8_H

#include <stm32f10x.h>
#include <stm32f10x_conf.h>
#include <adc.h>		    // случайное число
#include <stdlib.h>		    // srand, rand

uint8_t random_8(void);
uint8_t random_lim(uint8_t);
uint8_t random_min_max(uint8_t , uint8_t);
uint8_t qadd_8(uint8_t , uint8_t);
uint8_t qsub_8(uint8_t , uint8_t);
uint8_t scale_8_video(uint8_t , uint8_t);

#endif
