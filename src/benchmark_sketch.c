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

#include <libmeasure/measure.h>

#include "stream/stream.h"
#include "sketch/sketch.h"
#include "sketch/sketch_measure.h"
#include "util/xutil.h"

#define AMOUNT_OF_IMPLEMENTATIONS 2
#define IDX(STEP, x, y, z)  (z) + ((y) * (STEP)) + (N_EVENTS * (STEP) * (x))

typedef enum {
	MIN,
	MEDIAN,
} sketch_impl_t;

typedef struct {
	sketch_impl_t impl;
	uint32_t      index;
} alg_t;

static void printusage(char *argv[]) {
    fprintf(stderr, "Usage: %s \n"
            "\t[-o --output   [char *]   {REQUIRED} (Filename to write to)]\n"
            "\t[-f --file     [char *]   {REQUIRED} (Filename to hand to stream)]\n"
            "\t[-e --epsilon  [double]   {OPTIONAL} (Epsilon value)]\n"
            "\t[-d --delta    [double]   {OPTIONAL} (Delta value)]\n"
            "\t[-m --universe [uint32_t] {OPTIONAL} (Universe i.e. amount of unique items)]\n"
            "\t[-r --runs     [uint32_t] {OPTIONAL} (Amount of runs to average over)]\n"
            "\t[-w --width    [double]   {OPTIONAL} (Height of sketch)]\n"
            "\t[-h --height   [double]   {OPTIONAL} (Width of sketch)]\n"
            "\t[--min                    {OPTIONAL} (Run Count-Min Sketch)]\n"
            "\t[--median                 {OPTIONAL} (Run Count-Median Sketch)]\n"
            "\t[-1 --seed1    [uint32_t] {OPTIONAL} (First seed value)]\n"
            "\t[-2 --seed2    [uint32_t] {OPTIONAL} (Second seed value)]\n"
            "\t[-i --info                {OPTIONAL} (Shows this guideline)]\n"
            , argv[0]);
}

extern uint32_t N_EVENTS;

int main (int argc, char **argv) {
	uint32_t  i, j, k, k2, k3, uid;
	int32_t   opt;
	stream_t *stream;
	char      buf[256];
	char     *buffer;
	char     *filename = NULL;
	char     *output   = NULL;
	uint64_t  buf_size = 0;
	uint32_t  runs     = 5;
	bool      start    = true;

	alg_t                   alg[AMOUNT_OF_IMPLEMENTATIONS];
	sketch_measure_t        **impl;
	sketch_measure_create_t **params;

	uint8_t   impl_cnt = 0;
	double    epsilon  = 1./64.;
	double    delta    = 0.25;
	uint32_t  m        = UINT32_MAX;

	/* getopt */
	int option_index = 0;
	static int flag  = 0;
	static const char *optstring = "1:2:e:d:p:m:f:o:r:h:w:i";
	static const struct option long_options[] = {
		{"min",            no_argument, &flag,     MIN },
		{"median",         no_argument, &flag,  MEDIAN },
		{"epsilon",  required_argument,     0,      'e'},
		{"delta",    required_argument,     0,      'd'},
		{"universe", required_argument,     0,      'm'},
		{"file",     required_argument,     0,      'f'},
		{"output",   required_argument,     0,      'o'},
		{"runs",     required_argument,     0,      'r'},
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
				m = strtol(optarg, NULL, 10);
				break;
			case 'f':
				filename = strndup(optarg, 256);
				break;
			case 'o':
				output   = strndup(optarg, 256);
				break;
			case 'r':
				runs    = strtol(optarg, NULL, 10);
				break;
			case '1':
				I1 = strtoll(optarg, NULL, 10);
				break;
			case '2':
				I2 = strtoll(optarg, NULL, 10);
				break;
			case 'w':
				width = strtoll(optarg, NULL, 10);
				break;
			case 'h':
				depth = strtoll(optarg, NULL, 10);
				break;
			case 'i':
			default:
				printusage(argv);
				exit(EXIT_FAILURE);
		}
	}

	if ( NULL == filename || NULL == output ) {
		printusage(argv);
		exit(EXIT_FAILURE);
	}

	if ( impl_cnt == 0 ) {
		// This only work since the implementations appear first in long_options
		while (impl_cnt < AMOUNT_OF_IMPLEMENTATIONS) {
			alg[impl_cnt].impl  = long_options[impl_cnt].val;
			alg[impl_cnt].index = impl_cnt;
			impl_cnt++;
		}
	}

	printf("===========\n");
	printf("Parameters:\n");
	printf("===========\n");
	printf("m:       %"PRIu32"\n", m);
	printf("delta:   %lf\n", delta);
	printf("epsilon: %lf\n", epsilon);
	printf("runs:    %d\n",  runs);
	printf("===========\n\n");

	if ( !measure_init(output) ) {
		free(output);
		free(filename);
		xerror("Unable to initialize libmeasure", __LINE__, __FILE__);
	}

	stream = stream_open(filename);
	stream_set_data_size(stream, 1048576);

	impl   = xmalloc( sizeof(sketch_measure_t *) * 
			impl_cnt*N_EVENTS*runs);
	params = xmalloc( sizeof(sketch_measure_create_t *) * 
			impl_cnt*N_EVENTS*runs);

	sketch_measure_create_t params_min = {
		.b       = 2,
		.epsilon = epsilon,
		.delta   = delta,
		.f       = &countMin,
		.hash    = &multiplyShift,
	};
	sketch_measure_create_t params_median = {
		.b       = 4,
		.epsilon = epsilon,
		.delta   = delta,
		.f       = &countMedian,
		.hash    = &multiplyShift,
	};

	for (k = 0; k < impl_cnt; k++) {
		for (k2 = 0; k2 < N_EVENTS; k2++) {
			for (k3 = 0; k3 < runs; k3++) {
				impl[IDX(runs, k, k2, k3)] = NULL;
				switch (alg[k].impl) {
					case MIN:
						params[IDX(runs, k, k2, k3)] = &params_min;
						break;
					case MEDIAN:
						params[IDX(runs, k, k2, k3)] = &params_median;
						break;
					default:
						free(output);
						free(filename);
						xerror("Unknown sketch implementation.", __LINE__, __FILE__);
				}
			}
		}

		measure_with_sideeffects_and_values(
				filename, 
				(char *)long_options[alg[k].index].name, 
				"create", 
				(testfunc)sketch_measure_create, 
				(void **)&params[IDX(runs, k, 0, 0)], 
				runs,
				(void **)&impl[IDX(runs, k, 0, 0)]
		);
	}


	i = 0;
	j = 0;
	buffer = stream_read(stream);

	while ( unlikely(j < stream->data.length && buffer[j] == '#') ) {
		while ( likely(i < stream->data.length && buffer[i] != '\n') ) {
			i++;
			if ( unlikely(i == stream->data.length) ) {
				buffer = stream_read(stream);
				i      = 0;
				j      = 0;
			}
		}
		i++;
		if ( unlikely(i == stream->data.length) ) {
			buffer = stream_read(stream);
			i      = 0;
		}
		j = i;
	}

	j++;

	do {
		if (!start) {
			buffer = stream_read(stream);
		}

		i = stream->data.length;

		if ( buf_size > 0 && j == 0 ) {
			memcpy(&buf[buf_size], &buffer[j], sizeof(uint32_t)-buf_size);
			j += sizeof(uint32_t)-buf_size;

			uid = (uint32_t)((uint8_t)buf[3] << 24)
			    | (uint32_t)((uint8_t)buf[2] << 16)
			    | (uint32_t)((uint8_t)buf[1] << 8 )
				| (uint32_t)((uint8_t)buf[0]);

			for (k = 0; k < impl_cnt; k++) {
				for (k2 = 0; k2 < N_EVENTS; k2++) {
					for (k3 = 0; k3 < runs; k3++) {
						if (impl[IDX(runs, k, k2, k3)] == NULL) {
							continue;
						}
						impl[IDX(runs, k, k2, k3)]->params.i = uid;
						impl[IDX(runs, k, k2, k3)]->params.c = 1;
					}
				}

				measure_with_sideeffects(
						filename, 
						(char *)long_options[alg[k].index].name, 
						"update", 
						(testfunc)sketch_measure_update, 
						(void **)&impl[IDX(runs, k, 0, 0)],
						runs
				);
			}

			buf_size = 0;
		}

		while ( j+sizeof(uint32_t) <= i ) {
			uid = (uint32_t)((uint8_t)buffer[j+3] << 24)
			    | (uint32_t)((uint8_t)buffer[j+2] << 16)
			    | (uint32_t)((uint8_t)buffer[j+1] << 8 )
				| (uint32_t)((uint8_t)buffer[j]);

			for (k = 0; k < impl_cnt; k++) {
				for (k2 = 0; k2 < N_EVENTS; k2++) {
					for (k3 = 0; k3 < runs; k3++) {
						if (impl[IDX(runs, k, k2, k3)] == NULL) {
							continue;
						}
						impl[IDX(runs, k, k2, k3)]->params.i = uid;
						impl[IDX(runs, k, k2, k3)]->params.c = 1;
					}
				}
				measure_with_sideeffects(
						filename, 
						(char *)long_options[alg[k].index].name, 
						"update", 
						(testfunc)sketch_measure_update, 
						(void **)&impl[IDX(runs, k, 0, 0)],
						runs
				);
			}

			j += sizeof(uint32_t);
		}

		if ( i-j > 0 ) {
			buf_size = i-j;
			memset(buf, '\0', 256);
			memcpy(&buf, &buffer[j], buf_size);
		}

		j = 0;
		start = false;
	} while ( !stream_eof(stream) );

	for (i = 0; i < 1000; i++) {
		for (k = 0; k < impl_cnt; k++) {
			for (k2 = 0; k2 < N_EVENTS; k2++) {
				for (k3 = 0; k3 < runs; k3++) {
					if (impl[IDX(runs, k, k2, k3)] == NULL) {
						continue;
					}
					impl[IDX(runs, k, k2, k3)]->params.i = xuni_rand() * m;
					impl[IDX(runs, k, k2, k3)]->params.c = 0;
				}
			}
			measure_with_sideeffects(
					filename, 
					(char *)long_options[alg[k].index].name, 
					"query", 
					(testfunc)sketch_measure_point, 
					(void **)&impl[IDX(runs, k, 0, 0)],
					runs
			);
		}
	}

	for (k = 0; k < impl_cnt; k++) {
		for (k2 = 0; k2 < N_EVENTS; k2++) {
			for (k3 = 0; k3 < runs; k3++) {
				if (impl[IDX(runs, k, k2, k3)] == NULL) {
					continue;
				}
				sketch_measure_destroy(impl[IDX(runs, k, k2, k3)]);
			}
		}
	}

	free(params);
	free(impl);

	if ( !measure_destroy() ){
		xerror("Unable to close libmeasure", __LINE__, __FILE__);
	}

	stream_close(stream);
	free(filename);
	free(output);

	return EXIT_SUCCESS;
}
