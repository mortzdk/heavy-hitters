#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <errno.h>

#include "xutil.h"
#include "stream.h"

stream_t *stream_open(char const *filename) {
	int fd;
	int err;

	stream_t *s    = xmalloc( sizeof(stream_t) );
	s->buffer_size = BUFFER_SIZE;

	if ( (fd = open(filename, O_RDONLY)) < 0 ) {
		xerror(strerror(errno), __LINE__, __FILE__);
	}

	s->fd  = fd;
	s->eof = false;

	data_t * d = &s->data;
	d->type   = sizeof(char);
	d->size   = DATA_SIZE;
	d->length = 0;
	d->data   = xmalloc( d->type * d->size );

    /* Advise the kernel of our access pattern.  */
    err = posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL|POSIX_FADV_WILLNEED);

	if (err < 0) {
		xerror(strerror(err), __LINE__, __FILE__);
	}

	return s;
}

void stream_set_buffer_size(stream_t *s, uint32_t const buffer_size) {
	if ( buffer_size > 0 ) {
		s->buffer_size = buffer_size;
	}
}

void stream_set_data_size(stream_t *s, uint32_t const data_size) {
	if ( data_size >= s->buffer_size ) {
		s->data.size = data_size;
		s->data.data = xrealloc(s->data.data, s->data.size*s->data.type);
	}
}

void stream_set_data_type(stream_t *s, size_t type) {
	if (type > 0) {
		s->data.type = type;
		s->data.data = xrealloc(s->data.data, s->data.size*s->data.type);
	}
}

void *stream_read(stream_t *s){
	int      err = -1;
	uint32_t buf_size = s->buffer_size;
	data_t  *d        = &s->data;
	char    *data     = d->data;
	char     buf[buf_size];

	memset(d->data, '\0', d->size * d->type);
	d->length = 0;

	s->eof = false;

    while ( (d->length+buf_size <= d->size*d->type) && 
			(err = read(s->fd, buf, buf_size)) != 0 ) {
        if ( unlikely(err == -1) ) {
			xerror(strerror(errno), __LINE__, __FILE__);
		}

		memcpy(&data[d->length], buf, err);
		d->length += err;
    }

	if ( d->length < d->size*d->type && err != 0 ) {
		err = read(s->fd, buf, d->size*d->type - d->length);

        if ( unlikely(err == -1) ) {
			xerror(strerror(errno), __LINE__, __FILE__);
		}

		if ( likely(err > 0) ) {
			memcpy(&data[d->length], buf, err);
		}
	}

	if ( unlikely(err == 0) ) {
		if ( (err = lseek(s->fd, 0, SEEK_SET)) < 0 ) {
			xerror(strerror(errno), __LINE__, __FILE__);
		}
		s->eof = true;
	}

	return d->data;
}

bool stream_eof(stream_t *s) {
	return s->eof;
}

void stream_close(stream_t *s) {
	if ( NULL != s ) {
		if ( close(s->fd) < 0) {
			xerror(strerror(errno), __LINE__, __FILE__);
		}

		if ( NULL != s->data.data ) {
			free(s->data.data);
			s->data.data = NULL;
		}
		free(s);
		s = NULL;
	}
}
