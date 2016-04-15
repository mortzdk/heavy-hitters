#include <criterion/criterion.h>
#include <stdio.h>
#include <inttypes.h>

#include "util/qsort.h"

Test(qsort, sort_direction, .disabled=0) {
	int64_t c[5] = {1, 4, 3, 2, 5};

	quicksort((Item *) c, 0, 4);

	for (uint32_t i = 0; i < 5; i++) {
		cr_assert(i+1 == c[i]);
	}
}
