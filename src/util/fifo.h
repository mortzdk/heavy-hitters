#ifndef H_FIFO
#define H_FIFO

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint32_t elm;
	uint8_t  layer;
} elm_t;

typedef struct {
	elm_t    *data;
	uint32_t  size;
	uint32_t  count;
	uint32_t  head;
	uint32_t  tail;
	elm_t     res;
} fifo_t;

fifo_t *fifo_create(uint32_t size);

void fifo_destroy(fifo_t *fifo);

void fifo_push_back(fifo_t *fifo, uint32_t element, uint8_t layer);

elm_t *fifo_pop_front(fifo_t *fifo);

bool fifo_empty(fifo_t *fifo);

#endif
