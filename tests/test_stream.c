#include <criterion/criterion.h>
#include <stdio.h>

#include "stream/stream.h"

Test(stream, should_open_and_close_file, .disabled=0) {
	// File of size 4*buffer_size + 7
	stream_t *s = stream_open("datasets/tests/test01");
	cr_assert(NULL != s, "Expected an allocated structure");
	cr_assert(s->fd > 0, "Expected fd > %d got %d", 0, s->fd);
	stream_close(s);
}

Test(stream, should_use_data_size_equal_buffer_size, .disabled=0) {
	// File of size 4*buffer_size + 7
	stream_t *s = stream_open("datasets/tests/test01");

	stream_set_data_size(s, s->buffer_size);

	for (int i = 0; i < 4; i++) {
		stream_read(s);
		cr_assert(
				s->data.length == s->buffer_size, 
				"Expected %"PRIu32" got %"PRIu32, 
				s->data.size, 
				s->data.length
		);
	}

	stream_read(s);
	cr_assert(
			s->data.length == 7, 
			"Expected %"PRIu32" got %"PRIu32, 
			7, 
			s->data.length
	);

	cr_assert(stream_eof(s));

	stream_close(s);
}

Test(stream, should_change_buffer_size, .disabled=0) {
	// File of size 4*buffer_size + 7
	stream_t *s = stream_open("datasets/tests/test01");
	uint32_t buf_size = s->buffer_size/4;

	stream_set_buffer_size(s, buf_size);

	cr_assert(
			s->buffer_size == buf_size, 
			"Expected %"PRIu32" got %"PRIu32, 
			buf_size, 
			s->buffer_size
	);

	stream_set_data_size(s, s->buffer_size);

	for (int i = 0; i < 16; i++) {
		stream_read(s);
		cr_assert(
				s->data.length == s->buffer_size, 
				"Expected %"PRIu32" got %"PRIu32, 
				s->data.size, 
				s->data.length
		);
	}

	stream_read(s);
	cr_assert(
			s->data.length == 7, 
			"Expected %"PRIu32" got %"PRIu32, 
			7, 
			s->data.length
	);

	cr_assert(stream_eof(s));

	stream_close(s);
}

/**
 * TODO: Test skewed buffer_size
 *
Test(stream, should_change_buffer_size_not_pow_of_2, .disabled=0) {
	// File of size 4*buffer_size + 7
	stream_t *s = stream_open("datasets/tests/test01");
	uint32_t old_size = s->buffer_size;
	uint32_t buf_size = 1233;
	uint32_t i = 0;

	stream_set_buffer_size(s, buf_size);

	cr_assert(
			s->buffer_size == buf_size, 
			"Expected %"PRIu32" got %"PRIu32, 
			buf_size, 
			s->buffer_size
	);

	stream_set_data_size(s, s->buffer_size);

	for (i = 0; i < (old_size/buf_size)*4; i++) {
		stream_read(s);
		cr_assert(
				s->data.length == s->buffer_size, 
				"Expected %"PRIu32" got %"PRIu32, 
				s->data.size, 
				s->data.length
		);
	}

	stream_read(s);
	cr_assert(
			s->data.length == (old_size*4+7)-(i*buf_size), 
			"Expected %"PRIu32" got %"PRIu32, 
			(old_size*4+7)-i*buf_size, 
			s->data.length
	);

	cr_assert(stream_eof(s));

	stream_close(s);
}
*/

Test(stream, should_change_data_size, .disabled=0) {
	// File of size 4*buffer_size + 7
	stream_t *s = stream_open("datasets/tests/test01");
	uint32_t buf_size = s->buffer_size/4;

	stream_set_data_type(s, sizeof(uint16_t));

	stream_set_buffer_size(s, buf_size);

	cr_assert(
			s->buffer_size == buf_size, 
			"Expected %"PRIu32" got %"PRIu32, 
			buf_size, 
			s->buffer_size
	);

	stream_set_data_size(s, s->buffer_size);

	for (int i = 0; i < 8; i++) {
		stream_read(s);
		cr_assert(
				s->data.length == s->buffer_size*sizeof(uint16_t), 
				"Expected %"PRIu32" got %"PRIu32, 
				s->data.size, 
				s->data.length
		);
	}

	stream_read(s);
	cr_assert(
			s->data.length == 7, 
			"Expected %"PRIu32" got %"PRIu32, 
			7, 
			s->data.length
	);

	cr_assert(stream_eof(s));

	stream_close(s);
}

Test(stream, should_continously_read_file, .disabled=0) {
	// File of size 4*buffer_size + 7
	stream_t *s = stream_open("datasets/tests/test01");
	uint32_t buf_size = s->buffer_size/4;

	stream_set_data_type(s, sizeof(uint16_t));

	stream_set_buffer_size(s, buf_size);

	cr_assert(
			s->buffer_size == buf_size, 
			"Expected %"PRIu32" got %"PRIu32, 
			buf_size, 
			s->buffer_size
	);

	stream_set_data_size(s, s->buffer_size);

	for (int j = 0; j < 10; j++) {
		for (int i = 0; i < 8; i++) {
			stream_read(s);
			cr_assert(
					s->data.length == s->buffer_size*sizeof(uint16_t), 
					"Expected %"PRIu32" got %"PRIu32, 
					s->data.size, 
					s->data.length
			);
		}

		stream_read(s);
		cr_assert(
				s->data.length == 7, 
				"Expected %"PRIu32" got %"PRIu32, 
				7, 
				s->data.length
		);

		cr_assert(stream_eof(s));
	}

	stream_close(s);
}
