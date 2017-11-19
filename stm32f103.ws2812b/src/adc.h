#ifndef __ADC_H
#define __ADC_H

#include <stm32f10x.h>
#include <stm32f10x_conf.h>

void adc_rng_init(void);
uint16_t get_adc_value(void);
uint32_t adc_rng_get(void);

#endif
