#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include "cyu3os.h"
#include "cyu3system.h"

typedef struct
{
	uint8_t size;
	uint8_t front;
	uint8_t rear;
	uint8_t* data[16];
} circ_queue;

void queue_init(void);
uint8_t queue_enqueue(uint8_t* msg, uint32_t len);
uint8_t queue_dequeue(uint8_t* msg, uint32_t len);
uint8_t queue_full(void);
uint8_t queue_empty(void);

#endif
