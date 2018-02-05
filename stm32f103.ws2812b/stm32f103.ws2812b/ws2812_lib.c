#include <ws2812_lib.h>

bool ws2812_transmit = false;

static const uint8_t dim_curve[256] = 
{
	0, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6,
    6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8,
    8, 8, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11,
    11, 11, 12, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15,
    15, 15, 16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20,
    20, 20, 21, 21, 22, 22, 22, 23, 23, 24, 24, 25, 25, 25, 26, 26,
    27, 27, 28, 28, 29, 29, 30, 30, 31, 32, 32, 33, 33, 34, 35, 35,
    36, 36, 37, 38, 38, 39, 40, 40, 41, 42, 43, 43, 44, 45, 46, 47,
    48, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
    63, 64, 65, 66, 68, 69, 70, 71, 73, 74, 75, 76, 78, 79, 81, 82,
    83, 85, 86, 88, 90, 91, 93, 94, 96, 98, 99, 101, 103, 105, 107, 109,
    110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
    146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
    193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,	
};

/* simple delay counter to waste time, don't rely on for accurate timing */
void Delay(__IO uint32_t nCount)
{
	while (nCount--) {
	}
}

/* Transmit the frambuffer with buffersize number of bytes to the LEDs 
 * buffersize = (#LEDs / 16) * 24 */
void WS2812_sendbuf(uint32_t buffersize)
{		
	// transmission complete flag, indicate that transmission is taking place
	ws2812_transmit = true;
	
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

// Извлекает  нужный цвет в формате uint8_t из 32 битной переменной содержащей три цвета 0x00RRGGBB
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
// Хранится в самой старшей части старшего байта корректирующей цвет посылки.
rgb_operation eject_operation(uint32_t input_tone, rgb_mask color)
{
	rgb_operation result = 0x00;
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
		
	// проверяем на переполнение.
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
		
	// проверяем на переполнение.
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
		
	    // проверяем на переполнение.
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
		
	    // проверяем на переполнение.
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
		
	    // проверяем на переполнение.
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
		
	    // проверяем на переполнение.
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
	
	// высчитываем коэффициенты на которые будем прибавлять каждый раз
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

/*------------------------------------------------------------------------------
   Конвертируем RGB буффер в DMA буффер
 ------------------------------------------------------------------------------*/
void convert_rgb_to_dma_buf(uint32_t * buf)
{
	static uint8_t i;
	
	for (i = 0; i < NUMOFLEDS; i++)
	{		
		// раскомментить в зависимости от используемого канала.
	  //WS2812_framedata_setPixel(0, i, buf[i]);
	  //WS2812_framedata_setPixel(1, i, buf[i]);
	  //WS2812_framedata_setPixel(2, i, buf[i]);
	  //WS2812_framedata_setPixel(3, i, buf[i]);
		WS2812_framedata_setPixel(4, i, buf[i]);
		WS2812_framedata_setPixel(5, i, buf[i]);
		WS2812_framedata_setPixel(6, i, buf[i]);
		WS2812_framedata_setPixel(7, i, buf[i]);
	}
	
	WS2812_sendbuf(BUFFERSIZE);	
	
	// ждем пока окончится передача в DMA одного кадра.
	Delay(1500);
	
	// передача окончена
	ws2812_transmit = false;
}

/*------------------------------------------------------------------------------
  Корнвертер из HSV в RGB в целочисленной арифмерите
 
  hue        : 0..360
  saturation : 0..255
  value      : 0..255
 ------------------------------------------------------------------------------*/
uint32_t hsv_to_rgb(int hue, int sat, int val) 
{
	int    r;
	int    g;
	int    b;
	int    base;
	uint32_t rgb;

	val = dim_curve[val];
	sat = 255 - dim_curve[255 - sat];


	if (sat == 0) // Acromatic color (gray). Hue doesn't mind.
	{
		rgb = val | (val << 8) | (val << 16);
	}
	else
	{
		base = ((255 - sat) * val) >> 8;
		switch (hue / 60)
		{
		case 0:
			r = val;
			g = (((val - base) * hue) / 60) + base;
			b = base;
			break;
		case 1:
			r = (((val - base) * (60 - (hue % 60))) / 60) + base;
			g = val;
			b = base;
			break;
		case 2:
			r = base;
			g = val;
			b = (((val - base) * (hue % 60)) / 60) + base;
			break;
		case 3:
			r = base;
			g = (((val - base) * (60 - (hue % 60))) / 60) + base;
			b = val;
			break;
		case 4:
			r = (((val - base) * (hue % 60)) / 60) + base;
			g = base;
			b = val;
			break;
		case 5:
			r = val;
			g = base;
			b = (((val - base) * (60 - (hue % 60))) / 60) + base;
			break;
		}
		rgb = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
	}
	return rgb;
}

/*------------------------------------------------------------------------------
  Корнвертер из HSV в RGB с плавающей точкой
 
  hue        : 0..360
  saturation : 0..255
  value      : 0..255
------------------------------------------------------------------------------*/
uint32_t hsv_to_rgb_double(uint32_t hue, uint32_t saturation, uint32_t value) 
{
	uint8_t rgb[3]; 
	double r, g, b;
	
	double h =  (double)hue / 360.0;
	double s = (double)saturation / 255.0;
	double v = (double)value / 255.0;
	
	int i = (int)(h * 6);
	double f = h * 6 - i;
	double p = v * (1 - s);
	double q = v * (1 - f * s);
	double t = v * (1 - (1 - f) * s);

	switch (i % 6) {
	case 0: r = v, g = t, b = p; break;
	case 1: r = q, g = v, b = p; break;
	case 2: r = p, g = v, b = t; break;
	case 3: r = p, g = q, b = v; break;
	case 4: r = t, g = p, b = v; break;
	case 5: r = v, g = p, b = q; break;
	}

	rgb[0] = r * 255;
	rgb[1] = g * 255;
	rgb[2] = b * 255;
	return ((rgb[0] & 0xFF) << 16) | ((rgb[1] & 0xFF) << 8) | (rgb[2] & 0xFF);
}

/*------------------------------------------------------------------------------
  Перемещение светодиодов в заданную сторону с заданной задержкой и заданное количество шагов
------------------------------------------------------------------------------*/
void move_leds(uint32_t n, uint32_t dir, uint32_t del, uint32_t * buf)
{
	uint32_t i, k;
	uint32_t tmpLED;

	for (k = 0; k < n; k++)
	{
		convert_rgb_to_dma_buf(buf);

		if (dir == 0)
		{
		  // Перемещаем огни
			tmpLED = buf[0];
			for (i = 0; i < NUMOFLEDS; i++)
			{
				if (i != (NUMOFLEDS - 1))
				{
					buf[i] = buf[i + 1];
				}
				else
				{
					buf[i] = tmpLED;
				}
			}
		}
		else
		{
		  // Перемещаем огни
			tmpLED = buf[NUMOFLEDS - 1];
			for (i = 0; i < NUMOFLEDS; i++)
			{
				if (i != 0)
				{
					buf[NUMOFLEDS - i] = buf[NUMOFLEDS - i - 1];
				}
				else
				{
					buf[0] = tmpLED;
				}
			}
		}
		
		Delay(del);
	}
}

/*------------------------------------------------------------------------------
  Заполняем радугой 
 ------------------------------------------------------------------------------*/
void fill_rainbow(uint32_t sat, uint32_t val, uint8_t mode, uint32_t * buf)
{
	uint32_t i;
	uint32_t hue;

	    // Записываем в буфер начальную последовательность
	for (i = 0; i < NUMOFLEDS; i++)
	{
		hue = ((360 * i) / NUMOFLEDS);
		if (mode == 0)
		{
			buf[i] = hsv_to_rgb(hue, sat, val);
		}
		else
		{
			buf[i] = hsv_to_rgb_double(hue, sat, val);		
		}				
	}
}

/*------------------------------------------------------------------------------
  Бегущая радуга 
 ------------------------------------------------------------------------------*/
void running_rainbow(uint32_t * buf)
{
	while (1)
	{
		fill_rainbow(255, 255, 0, buf);
		move_leds(NUMOFLEDS, 1, 200000, buf);
		fill_rainbow(255, 255, 0, buf);
		move_leds(NUMOFLEDS, 1, 200000, buf);		
	}	
}