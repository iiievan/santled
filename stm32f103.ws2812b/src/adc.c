
#include <string.h>
#include <stm32f10x_adc.h>
#include <adc.h>


uint16_t get_adc_value(void)
{
    // Start the conversion
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	// Wait until conversion completion
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
		;
	// Get the conversion value
	return ADC_GetConversionValue(ADC1);
}

uint32_t adc_rng_get(void)
{
	uint32_t value = 0;
	uint8_t i;

	for (i = 0; i < 16; i++)
		value |= (get_adc_value() & 0x0003) << (i * 2);
	return value;
}


