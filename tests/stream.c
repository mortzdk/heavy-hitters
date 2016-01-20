#include <criterion/criterion.h>

#include "stream.h"

Test(stream, should_open_file, .disabled=0) {
	stream_t *s = stream_open();	
}
