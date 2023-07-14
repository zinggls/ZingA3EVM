#include "queue.h"
#include "ZingHw.h"

static circ_queue queue;

void queue_init(void)
{
	queue.front = 0;
	queue.rear = 0;
	queue.size = 0;

	for (uint8_t i = 0; i < 16; i++)
	{
		queue.data[i] = (uint8_t*)CyU3PDmaBufferAlloc(sizeof(uint8_t) * 512);
	}
}

uint8_t queue_enqueue(uint8_t* msg, uint32_t len)
{
	if (queue_full())
	{
		return 0;
	}
	else
	{
		queue.size += 1;
		queue.rear = (queue.rear + 1) % 16;
		memcpy(queue.data[queue.rear], msg, len);
		return 1;
	}
}

uint8_t queue_dequeue(uint8_t* msg, uint32_t len)
{
	if (queue_empty())
	{
		return 0;
	}
	else
	{
		queue.size -= 1;
		queue.front = (queue.front + 1) % 16;
		memcpy(msg, queue.data[queue.front], len);
		return 1;
	}
}

uint8_t queue_full(void)
{
	if (queue.size == 16)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t queue_empty(void)
{
	if (queue.size == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
