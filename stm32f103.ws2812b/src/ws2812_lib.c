#include <misc.h>
#include "ws2812_lib.h"

uint8_t WS2812_TC = 1;

uint16_t WS2812_IO_framedata[BUFFERSIZE];

/* Transmit the frambuffer with buffersize number of bytes to the LEDs 
 * buffersize = (#LEDs / 16) * 24 */
void WS2812_sendbuf(uint32_t buffersize)
{		
	// transmission complete flag, indicate that transmission is taking place
	WS2812_TC = 0;
	
	// clear all relevant DMA flags
	DMA_ClearFlag(DMA1_FLAG_TC2 | DMA1_FLAG_HT2 | DMA1_FLAG_GL2 | DMA1_FLAG_TE2);
	DMA_ClearFlag(DMA1_FLAG_TC5 | DMA1_FLAG_HT5 | DMA1_FLAG_GL5 | DMA1_FLAG_TE5);
	DMA_ClearFlag(DMA1_FLAG_HT7 | DMA1_FLAG_GL7 | DMA1_FLAG_TE7);
	
	// configure the number of bytes to be transferred by the DMA controller
	DMA_SetCurrDataCounter(DMA1_Channel2, buffersize);
	DMA_SetCurrDataCounter(DMA1_Channel5, buffersize);
	DMA_SetCurrDataCounter(DMA1_Channel7, buffersize);
	
	// clear all TIM2 flags
	TIM2->SR = 0;
	
	// enable the corresponding DMA channels
	DMA_Cmd(DMA1_Channel2, ENABLE);
	DMA_Cmd(DMA1_Channel5, ENABLE);
	DMA_Cmd(DMA1_Channel7, ENABLE);
	
	// IMPORTANT: enable the TIM2 DMA requests AFTER enabling the DMA channels!
	TIM_DMACmd(TIM2, TIM_DMA_CC1, ENABLE);
	TIM_DMACmd(TIM2, TIM_DMA_CC2, ENABLE);
	TIM_DMACmd(TIM2, TIM_DMA_Update, ENABLE);
	
	// preload counter with 29 so TIM2 generates UEV directly to start DMA transfer
	TIM_SetCounter(TIM2, 29);
	
	// start TIM2
	TIM_Cmd(TIM2, ENABLE);
}

/* This function sets the color of a single pixel in the framebuffer 
 * 
 * Arguments:
 * row = the channel number/LED strip the pixel is in from 0 to 15
 * column = the column/LED position in the LED string from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixel should display 
 */
void WS2812_framedata_setPixel(uint8_t row, uint16_t column, uint32_t rgb)
{
	uint8_t i;
	uint8_t red   = (uint8_t)((rgb & 0x00FF0000) >> 16);
	uint8_t green = (uint8_t)((rgb & 0x0000FF00) >> 8);
	uint8_t blue  = (uint8_t)(rgb & 0x000000FF);

	for (i = 0; i < 8; i++)
	{
		// clear the data for pixel 
		WS2812_IO_framedata[((column * 24) + i)] &= ~(0x01 << row);
		WS2812_IO_framedata[((column * 24) + 8 + i)] &= ~(0x01 << row);
		WS2812_IO_framedata[((column * 24) + 16 + i)] &= ~(0x01 << row);
		// write new data for pixel
		WS2812_IO_framedata[((column * 24) + i)] |= ((((green << i) & 0x80) >> 7) << row);
		WS2812_IO_framedata[((column * 24) + 8 + i)] |= ((((red << i) & 0x80) >> 7) << row);
		WS2812_IO_framedata[((column * 24) + 16 + i)] |= ((((blue << i) & 0x80) >> 7) << row);
	}
}

/* This function is a wrapper function to set all LEDs in the complete row to the specified color
 * 
 * Arguments:
 * row = the channel number/LED strip to set the color of from 0 to 15
 * columns = the number of LEDs in the strip to set to the color from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixels should display 
 */
void WS2812_framedata_setRow(uint8_t row, uint16_t columns, uint32_t rgb)
{
	uint8_t i;
	for (i = 0; i < columns; i++)
	{
		WS2812_framedata_setPixel(row, i, rgb);
	}
}

/* This function is a wrapper function to set all the LEDs in the column to the specified color
 * 
 * Arguments:
 * rows = the number of channels/LED strips to set the row in from 0 to 15
 * column = the column/LED position in the LED string from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixels should display 
 */
void WS2812_framedata_setColumn(uint8_t rows, uint16_t column, uint32_t rgb)
{
	uint8_t i;
	for (i = 0; i < rows; i++)
	{
		WS2812_framedata_setPixel(i, column, rgb);
	}
}

// »звлекает  нужный цвет в формате uint8_t из 32 битной переменной содержащей три цвета 0x00RRGGBB
uint8_t put_rgb_mask(uint32_t input_tone, rgb_mask color)
{	
	uint8_t output = 0x00;
	uint32_t input = input_tone; 
	
	switch (color)
	{
	case RED :
		output = (uint8_t)((input & 0x00FF0000) >> 16);
		break;
	case GREEN :
		output = (uint8_t)((input & 0x0000FF00) >> 8);
		break;
	case BLUE :
		output = (uint8_t)(input & 0x000000FF);
		break;
	default:
		break;
	}

	return output;
}

// извлекает тип операции с цветом: вычесть его или сложить. 
// ’ранитс€ в самой старшей части старшего байта корректирующей цвет посылки.
rgb_operation eject_operation(uint32_t input_tone, rgb_mask color)
{
	rgb_operation result = ADD;
	uint32_t input = input_tone;
	
	switch (color)
	{
	case RED :
		if ((input & 0x40000000) != 0)
		{
			result = ADD;	
		}
		else
		{
			result = SUB;
		}
		
		break;
		
	case GREEN :
		if ((input & 0x20000000) != 0)
		{
			result = ADD;	
		}
		else
		{
			result = SUB;
		}
		break;
		
	case BLUE :
		
		if ((input & 0x10000000) != 0)
		{
			result = ADD;	
		}
		else
		{
			result = SUB;
		}
		break;
		
	default:
		break;
	}
	
	return result;
}

uint32_t tone_correction_func(uint32_t input_tone, 
	uint8_t r_index,
	rgb_operation r_op,
	uint8_t g_index,
	rgb_operation g_op,
	uint8_t b_index,
	rgb_operation b_op)
{
	uint32_t out = 0x00000000;
	
	uint8_t red   = put_rgb_mask(input_tone, RED);
	uint8_t green = put_rgb_mask(input_tone, GREEN);
	uint8_t blue  = put_rgb_mask(input_tone, BLUE);
	
	switch (r_op)
	{
	case ADD:
		
	// провер€ем на переполнение.
		if ((red + r_index) <= 0xFF)
		{
			red += r_index;			
		}
		else
		{
			red = 0xFF;
		}

		break;
		
	case SUB:
		
	// провер€ем на переполнение.
		if ((red - r_index) >= 0x00)
		{
			red -= r_index;			
		}
		else
		{
			red = 0x00;
		}
		
		break;		
	
	default:
		break;
	}
	
	switch (g_op)
	{
	case ADD:
		
	    // провер€ем на переполнение.
		if ((green + g_index) <= 0xFF)
		{
			green += g_index;			
		}
		else
		{
			green = 0xFF;
		}

		break;
		
	case SUB:
		
	    // провер€ем на переполнение.
		if ((green - g_index) >= 0x00)
		{
			green -= g_index;			
		}
		else
		{
			green = 0x00;
		}
		
		break;		
	
	default:
		break;
	}
	
	switch (b_op)
	{
	case ADD:
		
	    // провер€ем на переполнение.
		if ((blue + b_index) <= 0xFF)
		{
			blue += b_index;			
		}
		else
		{
			blue = 0xFF;
		}

		break;
		
	case SUB:
		
	    // провер€ем на переполнение.
		if ((blue - b_index) >= 0x00)
		{
			blue -= b_index;			
		}
		else
		{
			blue = 0x00;
		}
		
		break;		
	
	default:
		break;
	}
	
	out |= ((uint32_t)red << 16);
	out |= ((uint32_t)green << 8);
	out |=  (uint32_t)blue;
	
	return out;
}


void fill_frame(frame_t * const temp_frame, uint32_t rgb, uint8_t scale)
{
	// иницилизируем указатель на пиксели.
	temp_frame->pixel = (uint8_t)NUMOFLEDS;
	
	uint8_t actual_frame_size = ((NUMOFLEDS * scale) / 10);
	uint32_t temp_color = 0;
	uint8_t red   = put_rgb_mask(rgb, RED);
	uint8_t green = put_rgb_mask(rgb, GREEN);
	uint8_t blue  = put_rgb_mask(rgb, BLUE);
	
	// высчитываем коэффициенты на которые будем прибавл€ть каждый раз
	uint8_t red_delta   =  red / actual_frame_size;
	uint8_t green_delta =  green / actual_frame_size;
	uint8_t blue_delta  =  blue / actual_frame_size;
	
	red   = 0xFF;
	green = 0xFF;
	blue  = 0xFF;
	
	for (uint8_t i = 0; i < NUMOFLEDS; i++)
	{
		temp_frame->pixels[i] = 0x00000000;
	}
	
	for (uint8_t i = 0; i < NUMOFLEDS; i++)
	{
		if (i <= actual_frame_size)
		{

			temp_color |= ((uint32_t)(red) << 16);
			temp_color |= ((uint32_t)(green) << 8);
			temp_color |=  (uint32_t)(blue);					
			
			temp_frame->pixels[i] = temp_color;
			
			red   = red - red_delta;
			green = green - green_delta;
			blue  = blue - blue_delta;
		}
		
		temp_color = 0;
		
		temp_frame->pixel = temp_frame->pixel - 1;
	}
}
