#include <criterion/criterion.h>
#include <stdio.h>

#include "stream.h"

Test(stream, should_open_file, .disabled=0) {
	// File of size 4*buffer_size + 7
	stream_t *s = stream_open("datasets/tests/test01");

	stream_set_data_size(s, s->buffer_size);

	printf("DATA Size: %"PRIu32"\n", s->data.size);

	stream_read(s);
	cr_assert(s->data.length == s->buffer_size, "Expected %"PRIu32" got %"PRIu32, s->data.size, s->data.length);

	stream_read(s);
	cr_assert(s->data.length == s->buffer_size, "Expected %"PRIu32" got %"PRIu32, s->data.size, s->data.length);

	stream_read(s);
	cr_assert(s->data.length == s->buffer_size, "Expected %"PRIu32" got %"PRIu32, s->data.size, s->data.length);

	stream_read(s);
	cr_assert(s->data.length == s->buffer_size, "Expected %"PRIu32" got %"PRIu32, s->data.size, s->data.length);

	stream_read(s);
	cr_assert(s->data.length == 7, "Expected %"PRIu32" got %"PRIu32, 7, s->data.length);

	cr_assert(stream_eof(s));

	stream_close(s);
}
