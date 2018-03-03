#include <main.h>


volatile uint8_t TIM2_overflows = 0;

static bool usart_rxtx = false;				// идет прием/передача по usart

static uint32_t usart_buffer_reset_tmr = 0;	// Таймер сброса Буффера, если получили не полные восемь байт

ring_buffer usart_buffer = { 0 };

struct CRGB leds_buf[NUMOFLEDS] = {0x00};	// буфер для бегущих эффектов.

// пиксель с начальным цветом.
struct CRGB rgb_coeff = { 0x03, 0x03, 0xff };
struct LIGHT_CMD light_cmd = { CMD_COLOR_CHOISE, CMD_HEAD_CONST, NO_BTN_CHANGE };


/* DMA1 Channel7 Interrupt Handler gets executed once the complete framebuffer has been transmitted to the LEDs */
void DMA1_Channel7_IRQHandler(void)
{
	// clear DMA7 transfer complete interrupt flag
	DMA_ClearITPendingBit(DMA1_IT_TC7);	
	// enable TIM2 Update interrupt to append 50us dead period
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	// disable the DMA channels
	DMA_Cmd(DMA1_Channel2, DISABLE);	
	DMA_Cmd(DMA1_Channel5, DISABLE);
	DMA_Cmd(DMA1_Channel7, DISABLE);
	// IMPORTANT: disable the DMA requests, too!
	TIM_DMACmd(TIM2, TIM_DMA_CC1, DISABLE);
	TIM_DMACmd(TIM2, TIM_DMA_CC2, DISABLE);
	TIM_DMACmd(TIM2, TIM_DMA_Update, DISABLE);
}

/* TIM2 Interrupt Handler gets executed on every TIM2 Update if enabled */
void TIM2_IRQHandler(void)
{
	// Clear TIM2 Interrupt Flag
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	
	/* check if certain number of overflows has occured yet 
	 * this ISR is used to guarantee a 50us dead time on the data lines
	 * before another frame is transmitted */
	if (TIM2_overflows < (uint8_t)WS2812_DEADPERIOD)
	{
		// count the number of occured overflows
		TIM2_overflows++;
	}
	else
	{
		// clear the number of overflows
		TIM2_overflows = 0;	
		// stop TIM2 now because dead period has been reached
		TIM_Cmd(TIM2, DISABLE);
		/* disable the TIM2 Update interrupt again 
		 * so it doesn't occur while transmitting data */
		TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
		// finally indicate that the data frame has been transmitted
	}
}
/*
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		// по сути сюда попадаем в случае если прием не получился или
		// получился мусор по USART и прошла 0.1 мс с момента приема последнего байта. 
		// в случае если посылка удалась, то тут делать нечего.
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);		
		
		// указатель в начало буфера (обнуляем буфер для приема новых данных)
		usart_buffer.head = usart_buffer.tail = 0;			
		// передача завершена, извещаем об этом всех и останавливаем таймер.
		usart_rxtx = false;
		TIM_Cmd(TIM3, DISABLE);	
		ResetIntervalTmr();
	}
}
*/

/*
 *  Функция обработчик прерывания USARTx.
 */
void USART1_IRQHandler(void)
{
	uint8_t rx_temp = 0xFF,
			bufer_status = RB_OK;    
	
	usart_sync_read(&rx_temp);	// получаем байт данных
	
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	
	// если приняли заголовок, тогда считаем байты - всего восемь.
	// общий формат сообщения 0A BC 40 FF 47 47 00 - для начального кадра (костер)
	// где 0А BC - это заголовок для смены цвета эффекта.
	// 0x40 - код операции, 0RGB 0000 - там где единица, там операция  сложить, где ноль вычесть.

	if (((rx_temp & 0x0F) == 0x0A && 
		 usart_buffer.head == 0) ||		// первый байт заголовка сообщения о корректировке
		(usart_buffer.head == 1 &&	    // второй байт заголовка сообщения о корректировке
		 rx_temp == 0xBC) ||
		usart_buffer.head > 1)
	{
			
		bufer_status = rb_write(&usart_buffer, &rx_temp, 1);	// пишем байт в кольцевой буффер	

		if (bufer_status != RB_OK)
		{				
			you_have_new_message = true;	// флаг для обновления цвета в лентах.			
			
			usart_buffer_reset_tmr = USART_BUFFER_RESET_TIME + 1;
			
			// передача завершена, извещаем об этом всех и останавливаем таймер.
			//TIM_Cmd(TIM3, DISABLE);	// Запускаем измерение интервала времени между посылками.
			//ResetIntervalTmr();
			usart_rxtx = false;
		}
		else
		{
			if (usart_rxtx != true)
			{
				//ResetIntervalTmr();
				//TIM_Cmd(TIM3, ENABLE);		// Запускаем измерение интервала времени между посылками.			
				
				usart_rxtx = true;	       //  идет прием пакета по usart.	
			}
			
			usart_buffer_reset_tmr = 0;
			//ResetIntervalTmr();
		}
	}	
}

int main(void) 
{	
	
  //static struct CRGB start_color = { 0xFB, 0xFB, 0xFB };
	//static struct CRGB end_color = { 0x00, 0xFF, 0x92 };
	
	low_level_init();
	
	rb_init(&usart_buffer);	// инициализируем буффер только перед инициализацией usart,
						    // там разрешается прерывание в конце.
	
	//you_have_new_message = true;	
	
	while (1) 
	{
		if (usart_buffer_reset_tmr <= USART_BUFFER_RESET_TIME)
		{
			usart_buffer_reset_tmr++;
		}
		else
		{
			// по истечение времени таймера останавливаем его и 
			// сравниваем хвост с головой, 
			usart_buffer_reset_tmr = USART_BUFFER_RESET_TIME + 1;
			usart_buffer.head = usart_buffer.tail = 0;
			usart_rxtx = false;
		}


		if (true == you_have_new_message)
		{	
			// сохраняем тип команды и кнопку меню.
			light_cmd.head = usart_buffer.storage[0];
			light_cmd.menu_button = usart_buffer.storage[2];
			
			// выделяем цвета из коэффициента что пришел по Bluetooth
			// и сохраняем их.		
			rgb_coeff.red   = usart_buffer.storage[3];
			rgb_coeff.green = usart_buffer.storage[4];
			rgb_coeff.blue  = usart_buffer.storage[5];					

			you_have_new_message = false;
			
			usart_buffer.head = usart_buffer.tail = 0;	// обнуляем буффер для приема следующего кадра.
		}
		
		switch (light_cmd.head)
		{
			case CMD_COLOR_CHOISE:
				{					
					uint8_t j;
					
					for (j = 0; j < NUMOFLEDS; j++)
					{		
						leds_buf[j].rgb = rgb_coeff.rgb;
					}
					
					convert_rgb_to_dma_buf(leds_buf);	
					
					Delay(100000);
				}
				break;
				
			case CMD_E_FIRE:
				{
					//random_lim(24);   // случайный  кадр из 24-х.
					//e_fire(leds_buf, true);
				}
				break;
				
			case CMD_RAINBOW:
			    {			
				    switch (light_cmd.menu_button)
				    {
						case NO_BTN_CHANGE:
						{
						    running_rainbow(leds_buf);   
						}
						   break;
						    
						case BTN_1:
						{
							rotating_rainbow(leds_buf);   
						}
						   break;
					    
						default:
						    break;
					    
				    }
				}
				break;
				
			case CMD_GRADIENT:
				{
				    // пока покадровый костер рулит, а это не очень то удачная попытка.
		            //gradient_fire(leds_buf, false, &start_color, &end_color);
					//gradient(leds_buf, false, &start_color, &end_color);
				}
				break;
				
			case CMD_CYCLE:
				{
					
				}
				break;
				
			default:
				break;
		}

		
		/*
		uint8_t i;	
		struct CHSV hsv_buf = { 0xff, 0xff, 0 };
	
        for (i = 0; i < NUMOFLEDS; i++)
		{		
			hsv2rgb(&hsv_buf, &leds_buf[i]);	
		}		
				
		convert_rgb_to_dma_buf(leds_buf);
		
		if (0xBF <= (++hsv_buf.hue))
		{
			hsv_buf.hue = 0;
		}
		*/
		
	}
}




