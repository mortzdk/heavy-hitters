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
#include "hh/hh.h"
#include "hh/hh_measure.h"
#include "hh/sketch.h"
#include "util/xutil.h"

#define AMOUNT_OF_IMPLEMENTATIONS 4
#define IDX(STEP, x, y, z)  (z) + ((y) * (STEP)) + (N_EVENTS * (STEP) * (x))

typedef enum {
	MIN,
	MEDIAN,
	CONST,
	CORMODE,
} hh_impl_t;

typedef struct {
	hh_impl_t impl;
	int       index;
} alg_t;

static void printusage(char *argv[]) {
    fprintf(stderr, "Usage (order is significant): %s \n"
            "\t[-o --output   [char *]   {REQUIRED} (Filename to write to)]\n"
            "\t[-f --file     [char *]   {REQUIRED} (Filename to hand to stream)]\n"
            "\t[-p --phi      [double]   {OPTIONAL} (Phi value)]\n"
            "\t[-m --universe [uint32_t] {OPTIONAL} (Universe i.e. amount of unique items)]\n"
            "\t[-e --epsilon  [double]   {OPTIONAL} (Epsilon value)]\n"
            "\t[-w --width    [double]   {OPTIONAL} (Height of sketch)]\n"
            "\t[-h --height   [double]   {OPTIONAL} (Width of sketch)]\n"
            "\t[-d --delta    [double]   {OPTIONAL} (Delta value)]\n"
            "\t[--min                    {OPTIONAL} (Run HH with Count Min Sketch)]\n"
            "\t[--median                 {OPTIONAL} (Run HH with Count Median Sketch)]\n"
            "\t[--const                  {OPTIONAL} (Run HH with Constant Count Min Sketch)]\n"
            "\t[--cormode                {OPTIONAL} (Run HH with Cormode et al.'s Count Min Sketch)]\n"
            "\t[-1 --seed1    [uint32_t] {OPTIONAL} (First seed value)]\n"
            "\t[-2 --seed2    [uint32_t] {OPTIONAL} (Second seed value)]\n"
            "\t[-r --runs     [uint32_t] {OPTIONAL} (Amount of runs to average over)]\n"
            "\t[-h --help                {OPTIONAL} (Shows this guideline)]\n"
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
	hh_measure_t          **impl;
	heavy_hitter_params_t **params;

	uint8_t   impl_cnt = 0;
	double    epsilon  = 1./64.;
	double    delta    = 0.25;
	double    phi      = 0.05;
	uint32_t  m        = UINT32_MAX;

	/* getopt */
	int option_index = 0;
	static int flag  = 0;
	static const char *optstring = "1:2:e:d:p:m:f:o:r:h:w:i";
	static const struct option long_options[] = {
		{"min",            no_argument, &flag,     MIN },
		{"median",         no_argument, &flag,  MEDIAN },
		{"const",          no_argument, &flag,   CONST },
		{"cormode",        no_argument, &flag, CORMODE },
		{"epsilon",  required_argument,     0,      'e'},
		{"delta",    required_argument,     0,      'd'},
		{"phi",      required_argument,     0,      'p'},
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
			case 'p':
				phi = strtod(optarg, NULL);
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
				epsilon = (double)2./width;
				break;
			case 'h':
				depth = strtoll(optarg, NULL, 10);
				delta = (2. * log2(m)) / (pow(2., depth)*phi); // branch = 2
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

	if ( epsilon > phi ) {
		free(filename);
		xerror("Epsilon cannot be bigger than phi", __LINE__, __FILE__);
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
	printf("phi:     %lf\n", phi);
	printf("runs:    %d\n", runs);
	printf("===========\n\n");

	if ( !measure_init(output) ) {
		free(output);
		free(filename);
		xerror("Unable to initialize libmeasure", __LINE__, __FILE__);
	}

	stream = stream_open(filename);
	stream_set_data_size(stream, 1048576);

	impl   = xmalloc( sizeof(hh_measure_t *) * 
			impl_cnt*N_EVENTS*runs);
	params = xmalloc( sizeof(heavy_hitter_params_t *) * 
			impl_cnt*N_EVENTS*runs);

	hh_sketch_params_t params_min = {
		.b       = 2,
		.epsilon = epsilon,
		.delta   = delta,
		.m       = m,
		.phi     = phi,
		.f       = &countMin,
	};
	hh_sketch_params_t params_median = {
		.b       = 4,
		.epsilon = epsilon,
		.delta   = delta,
		.m       = m,
		.phi     = phi,
		.f       = &countMedian,
	};

	heavy_hitter_params_t p_min = {
		.hash   = &multiplyShift,
		.params = &params_min,
		.f      = &hh_sketch,
	};
	heavy_hitter_params_t p_median = {
		.hash   = &multiplyShift,
		.params = &params_median,
		.f      = &hh_sketch,
	};
	heavy_hitter_params_t p_const = {
		.hash   = &multiplyShift,
		.params = &params_min,
		.f      = &hh_const_sketch,
	};
	heavy_hitter_params_t p_cormode = {
		.hash   = &multiplyShift,
		.params = &params_min,
		.f      = &hh_cormode_cmh,
	};

	for (k = 0; k < impl_cnt; k++) {
		for (k2 = 0; k2 < N_EVENTS; k2++) {
			for (k3 = 0; k3 < runs; k3++) {
				impl[IDX(runs, k, k2, k3)] = NULL;
				switch (alg[k].impl) {
					case MIN:
						params[IDX(runs, k, k2, k3)] = &p_min;
						break;
					case MEDIAN:
						params[IDX(runs, k, k2, k3)] = &p_median;
						break;
					case CONST:
						params[IDX(runs, k, k2, k3)] = &p_const;
						break;
					case CORMODE:
						params[IDX(runs, k, k2, k3)] = &p_cormode;
						break;
					default:
						free(output);
						free(filename);
						xerror("Unknown heavy hitter implementation.", __LINE__, __FILE__);
				}
			}
		}

		measure_with_sideeffects_and_values(
				filename, 
				(char *)long_options[alg[k].index].name, 
				"create", 
				(testfunc)heavy_hitter_measure_create, 
				(void **)&params[IDX(runs, k, 0, 0)], 
				runs,
				(void **)&impl[IDX(runs, k, 0, 0)]
		);
	}


	i = 0;
	j = 0;
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
						impl[IDX(runs, k, k2, k3)]->i = uid;
						impl[IDX(runs, k, k2, k3)]->c = 1;
					}
				}

				measure_with_sideeffects(
						filename, 
						(char *)long_options[alg[k].index].name, 
						"update", 
						(testfunc)heavy_hitter_measure_update, 
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
						impl[IDX(runs, k, k2, k3)]->i = uid;
						impl[IDX(runs, k, k2, k3)]->c = 1;
					}
				}
				measure_with_sideeffects(
						filename, 
						(char *)long_options[alg[k].index].name, 
						"update", 
						(testfunc)heavy_hitter_measure_update,
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
			measure_with_sideeffects(
					filename, 
					(char *)long_options[alg[k].index].name, 
					"query", 
					(testfunc)heavy_hitter_measure_query, 
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
				heavy_hitter_measure_destroy(impl[IDX(runs, k, k2, k3)]);
			}
		}
	}

	free(params);
	free(impl);

	if ( !measure_destroy() ){
		xerror("Unable to close libmeasure", __LINE__, __FILE__);
	}

	stream_close(stream);
	free(output);
	free(filename);

	return EXIT_SUCCESS;
}
