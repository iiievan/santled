#ifndef __MAIN_H
#define __MAIN_H

#pragma anon_unions	  // ��������� ������������� ����������

#include <stm32f10x_conf.h>
#include <stm32f10x.h>
#include <adc.h>		    // ��������� �����
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

// ��������� ������� � ������� ������, �������� �� Bluetooth
struct LIGHT_CMD {
	union {
		struct {
			union {
				uint8_t h;
				uint8_t head;	// ������� ����� ������� �� ������
			};
			union {
				uint8_t t;
				uint8_t topic;	// ������������ ��������� ������ �������
			};
			union {
				uint8_t b;
				uint8_t menu_button;	// ������� ������, ���������� ������� ����� �������
			};
		};
		
		uint32_t command;
	};
};

// �������� ���������� �������
typedef enum 
{
	CMD_COLOR_CHOISE      = 0x0A,	// ������ ������ ����� �� Bluetooth
	CMD_E_FIRE            = 0x1A,	// ������ "������������ ������"
	CMD_RAINBOW           = 0x2A,	// ������������ ������
	CMD_GRADIENT          = 0x3A,	// ������ ���������
	CMD_CYCLE             = 0x4A,	// ����������� ������� ���� ��������� ��������.
	CMD_HEAD_CONST        = 0xBC,	// ���������� ������������ ���������.
} light_cmd_head_t;

// �������� ���������� ������ ����
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

