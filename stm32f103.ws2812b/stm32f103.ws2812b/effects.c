#include <effects.h>


/*------------------------------------------------------------------------------
   Конвертируем RGB буффер в DMA буффер
 ------------------------------------------------------------------------------*/
static void convert_rgb_to_dma_buf(void)
{
	static uint8_t i;
	for (i = 0; i < NUMOFLEDS; i++)
	{	
		while (!WS2812_TC);		
		
		// раскомментить в зависимости от используемого канала.
	  //WS2812_framedata_setPixel(0, i, leds_buf[i]);
	  //WS2812_framedata_setPixel(1, i, leds_buf[i]);
	  //WS2812_framedata_setPixel(2, i, leds_buf[i]);
	  //WS2812_framedata_setPixel(3, i, leds_buf[i]);
		WS2812_framedata_setPixel(4, i, leds_buf[i]);
		WS2812_framedata_setPixel(5, i, leds_buf[i]);
		WS2812_framedata_setPixel(6, i, leds_buf[i]);
		WS2812_framedata_setPixel(7, i, leds_buf[i]);
	}
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
static void move_leds(uint32_t n, uint32_t dir, uint32_t del)
{
	uint32_t i, k;
	uint32_t tmpLED;

	for (k = 0; k < n; k++)
	{
		convert_rgb_to_dma_buf();

		if (dir == 0)
		{
		  // Перемещаем огни
			tmpLED = leds_buf[0];
			for (i = 0; i < NUMOFLEDS; i++)
			{
				if (i != (NUMOFLEDS - 1))
				{
					leds_buf[i] = leds_buf[i + 1];
				}
				else
				{
					leds_buf[i] = tmpLED;
				}
			}
		}
		else
		{
		  // Перемещаем огни
			tmpLED = leds_buf[NUMOFLEDS - 1];
			for (i = 0; i < NUMOFLEDS; i++)
			{
				if (i != 0)
				{
					leds_buf[NUMOFLEDS - i] = leds_buf[NUMOFLEDS - i - 1];
				}
				else
				{
					leds_buf[0] = tmpLED;
				}
			}
		}
		Delay(del);
	}
}

/*------------------------------------------------------------------------------
  Заполняем радугой 
 ------------------------------------------------------------------------------*/
static void fill_rainbow(uint32_t sat, uint32_t val, uint8_t mode)
{
	uint32_t i;
	uint32_t hue;

	    // Записываем в буфер начальную последовательность
	for (i = 0; i < NUMOFLEDS; i++)
	{
		hue = ((360 * i) / NUMOFLEDS);
		if (mode == 0)
		{
			leds_buf[i] = hsv_to_rgb(hue, sat, val);
		}
		else
		{
			leds_buf[i] = hsv_to_rgb_double(hue, sat, val);		
		}				
	}
}

/*------------------------------------------------------------------------------
  Бегущая радуга 
 ------------------------------------------------------------------------------*/
void running_rainbow(void)
{
	while (1)
	{
		fill_rainbow(255, 255, 0);
		move_leds(NUMOFLEDS * 2, 0, 20);
		fill_rainbow(255, 255, 1);
		move_leds(NUMOFLEDS * 2, 0, 20);
		
		
	}
}