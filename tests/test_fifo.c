#include <criterion/criterion.h>

#include "util/fifo.h"

Test(fifo, init_and_destroy, .disabled=0) {
	uint32_t size = 10;
	fifo_t *fifo  = fifo_create(size);

	cr_assert_not_null(fifo, "FIFO should not be NULL after creation");

	fifo_destroy(fifo);
}

Test(fifo, push_and_pop, .disabled=0) {
	elm_t *res;
	uint32_t size = 10;
	fifo_t *fifo  = fifo_create(size);

	cr_assert_not_null(fifo, "FIFO should not be NULL after creation");

	fifo_push_back(fifo, 0, 2);

	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 0, "Element: Expected %d got %d", 0, res->elm);
	cr_assert_eq(res->layer, 2, "Layer: Expected %d got %d", 2, res->layer);

	cr_assert(fifo_empty(fifo), "FIFO should be empty");

	fifo_destroy(fifo);
}

Test(fifo, realloc, .disabled=0) {
	elm_t *res;
	uint32_t size = 4;
	fifo_t *fifo  = fifo_create(size);

	cr_assert_not_null(fifo, "FIFO should not be NULL after creation");

	fifo_push_back(fifo, 0, 2);
	fifo_push_back(fifo, 1, 3);
	fifo_push_back(fifo, 2, 1);
	fifo_push_back(fifo, 3, 8);
	fifo_push_back(fifo, 4, 3);

	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 0, "Element: Expected %d got %d", 0, res->elm);
	cr_assert_eq(res->layer, 2, "Layer: Expected %d got %d", 2, res->layer);
	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 1, "Element: Expected %d got %d", 1, res->elm);
	cr_assert_eq(res->layer, 3, "Layer: Expected %d got %d", 3, res->layer);
	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 2, "Element: Expected %d got %d", 2, res->elm);
	cr_assert_eq(res->layer, 1, "Layer: Expected %d got %d", 1, res->layer);
	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 3, "Element: Expected %d got %d", 3, res->elm);
	cr_assert_eq(res->layer, 8, "Layer: Expected %d got %d", 8, res->layer);
	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 4, "Element: Expected %d got %d", 4, res->elm);
	cr_assert_eq(res->layer, 3, "Layer: Expected %d got %d", 3, res->layer);
	cr_assert(fifo_empty(fifo), "FIFO should be empty");

	fifo_destroy(fifo);
}

Test(fifo, realloc_middle, .disabled=0) {
	elm_t *res;
	uint32_t size = 4;
	fifo_t *fifo  = fifo_create(size);

	cr_assert_not_null(fifo, "FIFO should not be NULL after creation");

	fifo_push_back(fifo, 0, 2);
	fifo_push_back(fifo, 1, 3);
	fifo_push_back(fifo, 2, 1);

	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 0, "Element: Expected %d got %d", 0, res->elm);
	cr_assert_eq(res->layer, 2, "Layer: Expected %d got %d", 2, res->layer);
	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 1, "Element: Expected %d got %d", 1, res->elm);
	cr_assert_eq(res->layer, 3, "Layer: Expected %d got %d", 3, res->layer);
	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	fifo_push_back(fifo, 3, 8);
	fifo_push_back(fifo, 4, 3);
	fifo_push_back(fifo, 34, 0);
	fifo_push_back(fifo, 3333, 7);

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 2, "Element: Expected %d got %d", 2, res->elm);
	cr_assert_eq(res->layer, 1, "Layer: Expected %d got %d", 1, res->layer);
	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 3, "Element: Expected %d got %d", 3, res->elm);
	cr_assert_eq(res->layer, 8, "Layer: Expected %d got %d", 8, res->layer);
	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 4, "Element: Expected %d got %d", 4, res->elm);
	cr_assert_eq(res->layer, 3, "Layer: Expected %d got %d", 3, res->layer);
	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 34, "Element: Expected %d got %d", 34, res->elm);
	cr_assert_eq(res->layer, 0, "Layer: Expected %d got %d", 0, res->layer);
	cr_assert_not(fifo_empty(fifo), "FIFO should not be empty");

	res = fifo_pop_front(fifo);

	cr_assert_eq(res->elm, 3333, "Element: Expected %d got %d", 3333, res->elm);
	cr_assert_eq(res->layer, 7, "Layer: Expected %d got %d", 7, res->layer);
	cr_assert(fifo_empty(fifo), "FIFO should be empty");

	fifo_destroy(fifo);
}
