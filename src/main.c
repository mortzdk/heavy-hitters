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

#include "xutil.h"
#include "stream/stream.h"
#include "sketch/sketch.h"
#include "hh/hh.h"
#include "hh/sketch.h"

typedef struct {
	double   timestamp;
	uint16_t size;
	char     sourceIP[16];
	char     destinationIP[16];
	uint16_t sourcePort;
	uint16_t destinationPort;
	char     flags[5];
	uint8_t  protocol;
	uint8_t  direction;
	char     type[3];
	uint32_t random;
} NUST_t;

typedef enum {
	NUST,
} format_t;

void printusage(char *argv[]) {                                                 
    fprintf(stderr, "Usage: %s \n"                                              
            "\t[-f [char *] (Filename to hand to stream)]\n" 
            , argv[0]);                                                         
}

int main (int argc, char **argv) {
	uint32_t  i, j, uid;
	uint8_t   h1, h2, h3, h4;
	int32_t   opt, c;
	format_t  format;
	stream_t *stream;
	char     *string;
	char      buf[256];
	NUST_t    nust;
	char     *buffer;
	char     *filename  = NULL;
	char     *optstring = "e:d:p:m:f:n";
	uint64_t  buf_size = 0;

	double epsilon = 1./64.;
	double delta   = 0.25;
	double phi     = 0.05;
	uint32_t m     = UINT32_MAX;

	memset(nust.flags, '\0', 5);
	memset(nust.sourceIP, '\0', 16);
	memset(nust.destinationIP, '\0', 16);
	memset(nust.type, '\0', 3);

	while ((opt = getopt(argc, argv, optstring)) != -1) {
		switch (opt) {
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
				if ((NULL != strstr(filename, "NUST"))) {
					format = NUST;
				}
				break;
			default:
				printusage(argv);
				exit(EXIT_FAILURE);
		}
	}

	if ( NULL == filename ) {
		printusage(argv);
		exit(EXIT_FAILURE);
	}

	stream = stream_open(filename);
	stream_set_data_size(stream, 1048576);

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
		.params = &params_min
	};
	heavy_hitter_params_t p_median = {
		.hash   = &multiplyShift,
		.params = &params_median
	};

	hh_t *min        = heavy_hitter_create(&hh_sketch, &p_min);
	hh_t *median     = heavy_hitter_create(&hh_sketch, &p_median);
	hh_t *const_min  = heavy_hitter_create(&hh_const_sketch, &p_min);

	do {
		buffer = stream_read(stream);
		i = 0;
		j = 0;

		do {
			// Find end of line
			while ( i < stream->data.length && buffer[i] != '\n') {
				i++;
			}

			if ( i >= stream->data.length ) {
				break;
			}

			// If we have leftovers from last buffer
			if ( buf_size > 0 && j == 0 ) {
				memcpy(&buf[buf_size-1], &buffer[j], i-j);
				string = buf;
				buf_size = 0;
			} else {
				string = &buffer[j];
			}

			// Read NUST format
			switch (format) {
				case NUST:
					c = sscanf(
							string,
							"%lf %"SCNu16" %15s %15s %"SCNu16" %"SCNu16" %4s %"SCNu8" %"SCNu8" %2s %"SCNu32,
							&nust.timestamp,
							&nust.size,
							nust.sourceIP,
							nust.destinationIP,
							&nust.sourcePort,
							&nust.destinationPort,
							nust.flags,
							&nust.protocol,
							&nust.direction,
							nust.type,
							&nust.random
					);

					if ( unlikely(c < 11) ) {
						printf("c: %d\n", c);
						printf("%s\n", string);
						printf("last: %d\n", (string[i-j] == '\0'));
						printf("2nd last: %d\n", (string[i-j-1] == '3'));
						xerror("Unable to read NUST data", __LINE__, __FILE__);
					}

					sscanf(
							nust.sourceIP, 
							"%"SCNu8".%"SCNu8".%"SCNu8".%"SCNu8,
							&h1,
							&h2,
							&h3,
							&h4
					);

					if (h1 != 10) {
						uid = (uint32_t)(h1 << 24) | (h2 << 16) | (h3 << 8) | h4;
						heavy_hitter_update(min, uid, 1);
						heavy_hitter_update(median, uid, 1);
						heavy_hitter_update(const_min, uid, 1);
					}

					break;
				default:
					xerror("Invalid format received", __LINE__, __FILE__);
			}

			if ( unlikely(c == EOF && errno < 0) ) {
				xerror(strerror(errno), __LINE__, __FILE__);
			}

			i++;
			j = i;
		} while( 1 );

//		printf("Remaining: %s\n", &buffer[j]);
//		printf("i: %d\n", i);
//		printf("here: %d\n", (buffer[i] != '\n'));
		if (buffer[i] != '\n' && !stream_eof(stream)) {
			assert(i-j <= 256);

			buf_size = i-j;

			memset(buf, '\0', 256);
			memcpy(&buf, &buffer[j+1], buf_size);
		}

	} while (!stream_eof(stream));

	heavy_hitter_t *hitters_min       = heavy_hitter_query(min);

	printf("Heavy Hitters: COUNT-MIN:\n"); 
	for (i = 0; i < hitters_min->count; i++) {
		h1 = (hitters_min->hitters[i] & (0xff << 24)) >> 24;
		h2 = (hitters_min->hitters[i] & (0xff << 16)) >> 16;
		h3 = (hitters_min->hitters[i] & (0xff << 8)) >> 8;
		h4 = (hitters_min->hitters[i] & 0xff);
		printf("Heavy Hitter IP-Address: %"PRIu8".%"PRIu8".%"PRIu8".%"PRIu8"\n", 
				h1, h2, h3, h4);
	}

	heavy_hitter_t *hitters_median       = heavy_hitter_query(median);

	printf("Heavy Hitters: COUNT-MEDIAN:\n"); 
	for (i = 0; i < hitters_median->count; i++) {
		h1 = (hitters_median->hitters[i] & (0xff << 24)) >> 24;
		h2 = (hitters_median->hitters[i] & (0xff << 16)) >> 16;
		h3 = (hitters_median->hitters[i] & (0xff << 8)) >> 8;
		h4 = (hitters_median->hitters[i] & 0xff);
		printf("Heavy Hitter IP-Address: %"PRIu8".%"PRIu8".%"PRIu8".%"PRIu8"\n", 
				h1, h2, h3, h4);
	}

	heavy_hitter_t *hitters_const_min = heavy_hitter_query(const_min);
	printf("Heavy Hitters: CONST-COUNT-MIN:\n"); 
	for (i = 0; i < hitters_const_min->count; i++) {
		h1 = (hitters_const_min->hitters[i] & (0xff << 24)) >> 24;
		h2 = (hitters_const_min->hitters[i] & (0xff << 16)) >> 16;
		h3 = (hitters_const_min->hitters[i] & (0xff << 8)) >> 8;
		h4 = (hitters_const_min->hitters[i] & 0xff);
		printf("Heavy Hitter IP-Address: %"PRIu8".%"PRIu8".%"PRIu8".%"PRIu8"\n", 
				h1, h2, h3, h4);
	}

	heavy_hitter_destroy(min);
	heavy_hitter_destroy(median);
	heavy_hitter_destroy(const_min);

	stream_close(stream);

	return EXIT_SUCCESS;
}
