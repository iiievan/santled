#include <ring_buffer.h>
#include <string.h>

uint16_t rb_get_free_space(ring_buffer *buf) 
{
	if (buf->tail == buf->head)
		return RB_LENGTH - 1;

	if (buf->head > buf->tail)
		return RB_LENGTH - ((buf->head - buf->tail) + 1);
	else
		return (buf->tail - buf->head) - 1;
}

uint16_t rb_get_data_lenght(ring_buffer *buf) 
{
	return RB_LENGTH - (rb_get_free_space(buf) + 1);
}

void rb_init(ring_buffer *buf) 
{
	buf->head = buf->tail = 0;
	memset(buf->storage, 0, RB_LENGTH);
}

uint16_t rb_read(ring_buffer *buf, uint8_t *data, uint16_t len)
{
	uint16_t counter = 0;

	while (buf->tail != buf->head && counter < len) 
	{
		data[counter++] = buf->storage[buf->tail];
		buf->tail = (buf->tail + 1) % RB_LENGTH;
	}
	return counter;
}

uint8_t rb_write(ring_buffer *buf, uint8_t *data, uint16_t len) {
	uint16_t counter = 0;
	uint16_t free_space = rb_get_free_space(buf);

	if (free_space == 0)
		return RB_FULL;
	else if (free_space < len)
		return RB_NO_SPACE;

	while (counter < len)
	{
		buf->storage[buf->head] = data[counter++];
		buf->head = (buf->head + 1) % RB_LENGTH;
	}
	return RB_OK;
}