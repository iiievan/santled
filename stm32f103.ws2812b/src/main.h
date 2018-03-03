#ifndef __MAIN_H
#define __MAIN_H

#pragma anon_unions	  // разрешаем использование анонимусов

#include <stm32f10x_conf.h>
#include <stm32f10x.h>
#include <adc.h>		    // случайное число
#include <stdlib.h>		    // srand, rand
#include <ws2812_lib.h>
#include <ring_buffer.h>
#include <usart.h>
#include <ring_buffer.h>
#include <low_level_init.h>
#include <lib_8.h>
#include <stdbool.h>

/* this define sets the number of TIM2 overflows
 * to append to the data frame for the LEDs to 
 * load the received data into their registers */
#define WS2812_DEADPERIOD 19
#define USART_BUFFER_RESET_TIME 1000
#define ResetIntervalTmr() TIM_SetCounter(TIM3, 0)

// заголовок команды и нажатой кнопки, прин€той по Bluetooth
struct LIGHT_CMD {
	union {
		struct {
			union {
				uint8_t h;
				uint8_t head;	// отличие одной команды от другой
			};
			union {
				uint8_t t;
				uint8_t topic;	// об€зательный заголовок начала команды
			};
			union {
				uint8_t b;
				uint8_t menu_button;	// нажата€ кнопка, означающа€ вариант новой команды
			};
		};
		
		uint32_t command;
	};
};

// варианты заголовков команды
typedef enum 
{
	CMD_COLOR_CHOISE      = 0x0A,	// эффект выбора цвета по Bluetooth
	CMD_E_FIRE            = 0x1A,	// эффект "электронного костра"
	CMD_RAINBOW           = 0x2A,	// всевозможные радуги
	CMD_GRADIENT          = 0x3A,	// эффект градиента
	CMD_CYCLE             = 0x4A,	// циклический перебор всех доступных эффектов.
	CMD_HEAD_CONST        = 0xBC,	// посто€нна€ составл€юща€ заголовка.
} light_cmd_head_t;

// варианты включенных кнопок меню
typedef enum 
{
	NO_BTN_CHANGE = 0x00,
	BTN_1         = 0x01,
	BTN_2         = 0x02,
	BTN_3         = 0x04,
	BTN_4         = 0x08,
	BTN_5         = 0x10,
	BTN_6         = 0x20,
    BTN_7         = 0x40,
	BTN_8         = 0x80,
} menu_btn_t;

void DMA1_Channel7_IRQHandler(void);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void USART1_IRQHandler(void);
void UARTSend(const char *pucBuffer, uint32_t );


#endif // !_MAIN_H

