#ifndef RING_BUFFER_H__
#define RING_BUFFER_H__

#include <stdint.h>


#define RB_LENGTH 1000

typedef struct
{
	uint8_t storage[RB_LENGTH];
	uint8_t head, tail;
} ring_buffer;

typedef enum {
	RB_OK       = 0x0,
	RB_FULL,
	RB_NO_SPACE
} rb_status;

extern ring_buffer usart_buffer;

uint16_t rb_get_free_space(ring_buffer *buf);
uint16_t rb_get_data_lenght(ring_buffer *buf);
void rb_init(ring_buffer *buf);
uint16_t rb_read(ring_buffer *buf, uint8_t *data, uint16_t len);
uint8_t rb_write(ring_buffer *buf, uint8_t *data, uint16_t len);
uint32_t rb_parce(ring_buffer *buf, uint16_t len);



#endif //#ifndef RING_BUFFER_H__
