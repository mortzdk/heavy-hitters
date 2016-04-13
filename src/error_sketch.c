#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "stream/stream.h"
#include "sketch/sketch.h"
#include "util/xutil.h"

#define AMOUNT_OF_IMPLEMENTATIONS 2

typedef enum {
	MIN,
	MEDIAN,
} sketch_impl_t;

static void printusage(char *argv[]) {
    fprintf(stderr, "Usage: %s \n"
            "\t[-f --file     [char *]   {REQUIRED} (Filename to hand to stream)]\n"
            "\t[-m --universe [uint32_t] {OPTIONAL} (Universe i.e. amount of unique items)]\n"
            "\t[-e --epsilon  [double]   {OPTIONAL} (Epsilon value)]\n"
            "\t[-d --delta    [double]   {OPTIONAL} (Delta value)]\n"
            "\t[-w --width    [double]   {OPTIONAL} (Height of sketch)]\n"
            "\t[-h --height   [double]   {OPTIONAL} (Width of sketch)]\n"
            "\t[--min                    {OPTIONAL} (Run HH with Count Min Sketch)]\n"
            "\t[--median                 {OPTIONAL} (Run HH with Count Median Sketch)]\n"
            "\t[-1 --seed1    [uint32_t] {OPTIONAL} (First seed value)]\n"
            "\t[-2 --seed2    [uint32_t] {OPTIONAL} (Second seed value)]\n"
            "\t[-i --info                {OPTIONAL} (Shows this guideline)]\n"
            , argv[0]);
}

typedef struct {
	sketch_impl_t impl;
	int32_t       index;
} alg_t;

int int64comp(const void * a, const void * b) {
   return (int)(*(int64_t*)a - *(int64_t*)b);
}

int main (int argc, char **argv) {
	char      buf[256];
	char     *buffer;
	uint32_t  i, j, k, uid;
	uint64_t  *exact;
	uint64_t  *diffs;
	uint64_t  L1, L2;
	int64_t   point, diff;
	stream_t *stream;
	int32_t   opt;
	char     *filename = NULL;
	uint64_t  buf_size = 0;
	bool start         = true;

	uint64_t  error;
	alg_t     alg[AMOUNT_OF_IMPLEMENTATIONS];
	sketch_t *impl[AMOUNT_OF_IMPLEMENTATIONS];
	uint8_t   impl_cnt = 0;
	double    epsilon  = 1./64.;
	double    delta    = 0.25;
	uint32_t  m        = UINT32_MAX;

	/* getopt */
	int option_index = 0;
	static int flag  = 0;
	static const char *optstring = "1:2:e:d:m:f:h:w:i";
	static const struct option long_options[] = {
		{"min",            no_argument, &flag,     MIN },
		{"median",         no_argument, &flag,  MEDIAN },
		{"epsilon",  required_argument,     0,      'e'},
		{"delta",    required_argument,     0,      'd'},
		{"universe", required_argument,     0,      'm'},
		{"file",     required_argument,     0,      'f'},
		{"info",           no_argument,     0,      'i'},
        {"seed1",    required_argument,     0,      '1'},
        {"seed2",    required_argument,     0,      '2'},
        {"width",    required_argument,     0,      'w'},
        {"height",   required_argument,     0,      'h'},
	};

	while ((opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1) {
		switch (opt) {
			case 0:
				alg[impl_cnt].impl  = flag;
				alg[impl_cnt].index = option_index;
				impl_cnt++;
				break;
			case 'e':
				epsilon = strtod(optarg, NULL);
				break;
			case 'd':
				delta = strtod(optarg, NULL);
				break;
			case 'm':
				m = strtoll(optarg, NULL, 10);
				break;
			case 'f':
				filename = strndup(optarg, 256);
				break;
			case '1':
				I1 = strtoll(optarg, NULL, 10);
				break;
			case '2':
				I2 = strtoll(optarg, NULL, 10);
				break;
			case 'w':
				width = strtoll(optarg, NULL, 10);
				epsilon = (double)2./width;
				break;
			case 'h':
				depth = strtoll(optarg, NULL, 10);
				delta = 1./pow(2., depth);
				break;
			case 'i':
			default:
				printusage(argv);
				exit(EXIT_FAILURE);
		}
	}

	if ( NULL == filename ) {
		printusage(argv);
		exit(EXIT_FAILURE);
	}

	exact = xmalloc( m * sizeof(uint64_t) );
	memset(exact, '\0', m*sizeof(uint64_t)) ;

	// This only work since the implementations appear first in long_options
	if ( impl_cnt == 0 ) {
		while (impl_cnt < AMOUNT_OF_IMPLEMENTATIONS) {
			alg[impl_cnt].impl  = long_options[impl_cnt].val;
			alg[impl_cnt].index = impl_cnt;
			impl_cnt++;
		}
	}

	printf("#===========\n");
	printf("#Parameters:\n");
	printf("#===========\n");
	printf("#seed1:    %"PRIu32"\n", I1);
	printf("#seed2:    %"PRIu32"\n", I2);
	printf("#===========\n");

	stream = stream_open(filename);
	stream_set_data_size(stream, 1048576);

	for (k = 0; k < impl_cnt; k++) {
		switch (alg[k].impl) {
			case MIN:
				impl[k] = sketch_create(&countMin, &multiplyShift, 2, epsilon, 
						delta);
				break;
			case MEDIAN:
				impl[k] = sketch_create(&countMedian, &multiplyShift, 4, 
						epsilon, delta);
				break;
			default:
				stream_close(stream);
				free(filename);
				xerror("Unknown sketch implementation.", __LINE__, __FILE__);
		}
	}

#ifdef PRINT
	printf("#Reading Stream");
#endif

	i      = 0;
	j      = 0;
	buffer = stream_read(stream);

	// Skip comments
	while ( unlikely(j < stream->data.length && buffer[j] == '#') ) {
		// Read line until newline appears
		while ( likely(i < stream->data.length && buffer[i] != '\n') ) {
			i++;
			// If we run out of buffer we read a new chunk
			if ( unlikely(i == stream->data.length) ) {
				buffer = stream_read(stream);
				i      = 0;
				j      = 0;
			}
		}

		// Goto next character
		i++;

		// If we run out of buffer we read a new chunk
		if ( unlikely(i >= stream->data.length) ) {
			buffer = stream_read(stream);
			i      = 0;
		}

		if ( unlikely(buffer[i] == '\n') ) {
			i++;
			j = i;
			break;
		}

		j = i;
	}

	// Start working on data
	do {
#ifdef PRINT
		printf(".");
		fflush(stdout);
#endif
		if (!start) {
			buffer = stream_read(stream);
		}

		i = stream->data.length;

		if ( unlikely(buf_size > 0 && j == 0) ) {
			memcpy(&buf[buf_size], &buffer[j], sizeof(uint32_t)-buf_size);
			j += sizeof(uint32_t)-buf_size;

			uid = (uint32_t)((uint8_t)buf[3] << 24)
			    | (uint32_t)((uint8_t)buf[2] << 16)
			    | (uint32_t)((uint8_t)buf[1] << 8 )
				| (uint32_t)((uint8_t)buf[0]);

			exact[uid] += 1;

			for (k = 0; k < impl_cnt; k++) {
				sketch_update(impl[k], uid, 1);
			}

			buf_size = 0;
		}

		while ( likely(j+sizeof(uint32_t) <= i) ) {
			uid = (uint32_t)((uint8_t)buffer[j+3] << 24)
			    | (uint32_t)((uint8_t)buffer[j+2] << 16)
			    | (uint32_t)((uint8_t)buffer[j+1] << 8 )
				| (uint32_t)((uint8_t)buffer[j]);

			exact[uid] += 1;

			for (k = 0; k < impl_cnt; k++) {
				sketch_update(impl[k], uid, 1);
			}

			j += sizeof(uint32_t);
		}

		if ( likely(i-j > 0) ) {
			buf_size = i-j;
			memset(buf, '\0', 256);
			memcpy(&buf, &buffer[j], buf_size);
		}

		j      = 0;
		start = false;
	} while ( !stream_eof(stream) );

#ifdef PRINT
	printf("\n\nCalculating precision..\n\n");
#endif

	// Calculate norms
	L1 = 0;
	L2 = 0;
	for (i = 0; i < m; i++) {
		L1 += exact[i]; 
		L2 += (uint64_t)powl((long double)exact[i], 2.);
	}
	L2 = (uint64_t)sqrtl((long double)L2);

	diffs = xmalloc( m * sizeof(uint64_t) );
	memset(diffs, '\0', m*sizeof(uint64_t)) ;

	for (k = 0; k < impl_cnt; k++) {
		for (i = 0; i < m; i++) {
			point    = sketch_point(impl[k], i);
			diff     = (int64_t)point - (int64_t)exact[i];
			diffs[i] = llabs(diff);

#ifdef PRINT
	printf("Point: %"PRIi64"\n", point);
	printf("Exact: %"PRIi64"\n", exact[i]);
	printf("Error: %"PRIu64"\n", diffs[i]);
#endif
		}

		qsort(diffs, m, sizeof(int64_t), int64comp);

		assert(delta != 1);
		error = diffs[m - (uint32_t)(delta * m) -1];

		printf("%s,", long_options[alg[k].index].name);
		printf("%0.10lf,", (double)error);
		printf("%0.10lf,", epsilon);
		printf("%0.10lf,", delta);
		printf("%"PRIu32",", width);
		printf("%"PRIu32",", depth);
		printf("%"PRIu32",", m);
		printf("%"PRIu64",", L1);
		printf("%"PRIu64"\n", L2);
	}


	free(exact);
	stream_close(stream);
	free(filename);
}
