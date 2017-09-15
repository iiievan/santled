
#include <string.h>
#include <stm32f10x_adc.h>
#include <adc.h>


void adc_rng_init(void)
{
	ADC_InitTypeDef ADC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_InitStructure.ADC_NbrOfChannel          = ADC_Channel_12;
	ADC_InitStructure.ADC_Mode                  = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode          = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode    = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv      = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign             = ADC_DataAlign_Right;
	ADC_Init(ADC1, &ADC_InitStructure);


	//  АЦП берем по внутренней опоре Vrefint - это 17-й канал.
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 1, ADC_SampleTime_7Cycles5);

	ADC_Cmd(ADC1, ENABLE);
}

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


