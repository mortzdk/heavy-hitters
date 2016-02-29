#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h> 
#include <stdbool.h> 
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>

#include <papi.h>

#include <time.h>

#include "measure.h"

__attribute__ ((visibility("default"))) int N_EVENTS = 11;
__attribute__ ((visibility("default"))) extern void ***cleanup;
int          **events;

__attribute__ ((visibility("default")))
int measure(char *test, char *name, char *size, testfunc fp, void *up, int ups) {
	int res; 
	uint8_t  i, j;
	uint64_t total_ns, total_s;
	struct timespec begin, end;
	bool timemeasured = false; 
	long long meas[2] = {0, 0};

	printf("%s,%s,%s", test, name, size);

	for (i = 0; i < N_EVENTS; i++) {
		if (!timemeasured) {
			clock_gettime(CLOCK_REALTIME, &begin);
		}

		if ((res = PAPI_start_counters(&events[i][1], events[i][0])) != PAPI_OK) {
			for (j = 1; j <= events[i][0]; j++) {
				printf(",0");
			}
			continue;
		};

		for (j = 0; j < ups; j++) {
			(fp)(up);
		}

		PAPI_stop_counters(meas, events[i][0]);

		if (!timemeasured) {
			clock_gettime(CLOCK_REALTIME, &end);

			if (begin.tv_nsec <= end.tv_nsec) {
				total_ns = end.tv_nsec - begin.tv_nsec;
				total_s  = end.tv_sec  - begin.tv_sec;
			} else {
				total_ns = end.tv_nsec + (1e9 - begin.tv_nsec);
				total_s  = end.tv_sec - begin.tv_sec - 1;
			}

			uint64_t total = total_s * 1e9 + total_ns;
			printf(",%f", (double)total/ups);
			timemeasured = true;
		}

		for (j = 1; j <= events[i][0]; j++) {
			printf(",%lld", meas[j-1]/ups);
		}
	}

	printf("\n");

	return 0;
}

__attribute__ ((visibility("default")))
int measure_clean(char *test, char *name, char *size, testfunc fp, void ***up, int ups) {
	int res;
	uint8_t i, j;
	uint64_t total_ns, total_s;
	struct timespec begin, end;
	bool timemeasured = false;
	long long meas[2] = {0, 0};

	printf("%s,%s,%s", test, name, size);

	for (i = 0; i < N_EVENTS; i++) {
		if (!timemeasured) {
			clock_gettime(CLOCK_REALTIME, &begin);
		}

		if ((res = PAPI_start_counters(&events[i][1], events[i][0])) != PAPI_OK) {
			for (j = 1; j <= events[i][0]; j++) {
				printf(",0");
			}
			continue;
		};

		for (j = 0; j < ups; j++) {
			cleanup[i][j] = (fp)(up[i][j]);
		}

		PAPI_stop_counters(meas, events[i][0]);

		if (!timemeasured) {
			clock_gettime(CLOCK_REALTIME, &end);

			if (begin.tv_nsec <= end.tv_nsec) {
				total_ns = end.tv_nsec - begin.tv_nsec;
				total_s  = end.tv_sec  - begin.tv_sec;
			} else {
				total_ns = end.tv_nsec + (1e9 - begin.tv_nsec);
				total_s  = end.tv_sec - begin.tv_sec - 1;
			}

			uint64_t total = total_s * 1e9 + total_ns;
			printf(",%f", (double)total/ups);
			timemeasured = true;
		}

		for (j = 1; j <= events[i][0]; j++) {
			printf(",%lld", meas[j-1]/ups);
		}
	}

	printf("\n");

	return 0;
}

__attribute__ ((visibility("default")))
int
measure_init() {
	int res;
	uint8_t i, j;
	char buf[1024];
	
	events = malloc(N_EVENTS*sizeof(int *));

	if (events == NULL) {
		fprintf(stderr, "Error %d@%s: Unable to allocate PAPI events\n", __LINE__, __FILE__);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < N_EVENTS; i++) {
		events[i] = calloc(sizeof(int), 6);
		if (events[i] == NULL) {
			return 0;
		}
	}

	events[0][0] = 2;
	events[0][1] = PAPI_TOT_INS;
	events[0][2] = PAPI_TOT_CYC;

	events[1][0] = 2;
	events[1][1] = PAPI_BR_NTK;
	events[1][2] = PAPI_BR_TKN;

	events[2][0] = 2;
	events[2][1] = PAPI_BR_MSP;
	events[2][2] = PAPI_BR_PRC;

	events[3][0] = 2;
	events[3][1] = PAPI_L1_DCM;
	events[3][2] = PAPI_L1_ICM;

	events[3][0] = 2;
	events[3][1] = PAPI_L1_DCM;
	events[3][2] = PAPI_L1_ICM;

	events[4][0] = 2;
	events[4][1] = PAPI_L2_DCM;
	events[4][2] = PAPI_L2_ICM;

	events[5][0] = 2;
	events[5][1] = PAPI_TLB_DM;
	events[5][2] = PAPI_TLB_IM;

	events[6][0] = 1;
	events[6][1] = PAPI_L1_DCH;

	events[7][0] = 1;
	events[7][1] = PAPI_L2_DCH;

	events[8][0] = 1;
	events[8][1] = PAPI_L1_ICH;

	events[9][0] = 1;
	events[9][1] = PAPI_L2_ICH;

	events[10][0] = 1;
	events[10][1] = PAPI_REF_CYC;


	if ( (res = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
		fprintf(stderr, "Unable to initialize PAPI: %s\n", PAPI_strerror(res));
		return 0;
	}

	printf("Test,Name,Size,Nanoseconds");
	for (i = 0; i < N_EVENTS; i++) {
		for (j = 1; j <= events[i][0]; j++) {
			PAPI_event_code_to_name(events[i][j], buf);
			printf(",%s", buf);
		}
	}
	printf("\n");

	return 1;
}

__attribute__ ((visibility("default")))
void measure_destroy() {
	uint8_t i;

	for (i = 0; i < N_EVENTS; i++) {
		if (NULL != events[i]) {
			free(events[i]);
		}
	}

	free(events);

	PAPI_shutdown();
}
