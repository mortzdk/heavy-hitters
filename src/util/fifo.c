#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "fifo.h"
#include "xutil.h"

fifo_t *fifo_create(uint32_t size) {
	fifo_t *fifo = xmalloc( sizeof(fifo_t) );

	size         = next_pow_2(size);

	fifo->size   = size;
	fifo->data   = xmalloc( size * sizeof(elm_t) );
	fifo->count  = fifo->head = fifo->tail = 0;

	memset(fifo->data, '\0', size * sizeof(elm_t));

	return fifo;
}

void fifo_destroy(fifo_t *fifo) {
	if ( NULL != fifo ) {
		if ( NULL != fifo->data ) {
			free(fifo->data);
			fifo->data = NULL;
		}
		free(fifo);
		fifo = NULL;
	}
}

static inline void fifo_realloc(fifo_t *fifo) {
	uint32_t newsize;
	uint32_t size = fifo->size;
	uint32_t tail = fifo->tail;
	uint32_t head = fifo->head;

	if ( unlikely(fifo->count == size) ) {
		newsize    = 2*size;

		fifo->data = xrealloc(fifo->data, newsize*sizeof(elm_t));
		fifo->size = newsize;

		memset(fifo->data+size, '\0', size*sizeof(elm_t));

		if (tail <= head) {
			memcpy(fifo->data+(newsize-(size-head)), fifo->data+head, 
					(size-head)*sizeof(elm_t));
			fifo->head = newsize-(size-head);
		}
	}
}

void fifo_push_back(fifo_t *fifo, uint32_t element, uint8_t layer) {
	uint32_t tail = fifo->tail;

	fifo->data[tail].elm   = element;
	fifo->data[tail].layer = layer;
	fifo->tail             = (tail+1) & (fifo->size-1);

	fifo->count++;

	// Reallocate fifo if we does not have enough space
	fifo_realloc(fifo);
}

elm_t *fifo_pop_front(fifo_t *fifo) {
	uint32_t head;

	// The buffer is empty
	if ( unlikely(fifo->count == 0) ) {
		return NULL;
	}

	head = fifo->head;

	fifo->res.elm   = fifo->data[head].elm;
	fifo->res.layer = fifo->data[head].layer;
	fifo->head      = (head+1) & (fifo->size-1);

	fifo->count--;

	return &fifo->res;
}

bool fifo_empty(fifo_t *fifo) {
	return (fifo->count == 0) ? true : false;
}
