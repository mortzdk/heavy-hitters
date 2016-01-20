#ifndef H_stream
#define H_stream

#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

typedef struct {
	size_t   type;
	uint32_t size;
	void    *data;
} data_t;

typedef struct {
	int32_t  fd;
	uint32_t buffer_size;
	bool     eof;
	data_t   data;
} stream_t;

static const uint32_t BUFFER_SIZE = 16*1024;
static const uint32_t DATA_SIZE   = 1073741824; // 1 GB


stream_t *stream_open(char const *filename);

void stream_set_buffer_size(stream_t *s, uint32_t const buffer_size);

void stream_set_data_size(stream_t *s, uint32_t const data_size);

void stream_set_type(stream_t *s, size_t type);

char *stream_read(stream_t *s);

void stream_close(stream_t *s);

#endif
