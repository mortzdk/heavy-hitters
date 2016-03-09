#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <inttypes.h>
#include <errno.h>

#include "xutil.h"
#include "alias.h"

#define BUFFER (1024*512)
#define TOPK 1024


static void printusage(char *argv[]) {
    fprintf(stderr, "Usage: %s \n"
            "\t[-f --file     [char *]   {REQUIRED} (Filename to write to)]\n"
            "\t[-a --alpha    [double]   {OPTIONAL} (Alpha value)]\n"
            "\t[-c --count    [uint64_t] {OPTIONAL} (Times to sample)]\n"
            "\t[-N --elements [uint32_t] {OPTIONAL} (Amount of elements with mass)]\n"
            "\t[-m --universe [uint32_t] {OPTIONAL} (Amount of elements in universe)]\n"
            "\t[-1 --seed1    [uint32_t] {OPTIONAL} (First seed value)]\n"
            "\t[-2 --seed2    [uint32_t] {OPTIONAL} (Second seed value)]\n"
            "\t[-h --help                {OPTIONAL} (Shows this guideline)]\n"
            , argv[0]);
}

int main (int argc, char**argv) {
	FILE *file;
	uint64_t j, i = 0;
	uint32_t *res;
	uint32_t *map;
	double   *table;
	double c = 0;

	/* getopt */
	int opt;
    int option_index = 0;
    static const char *optstring = "1:2:c:N:a:f:m:h";
    static const struct option long_options[] = {
        {"alpha",    required_argument,     0,    'a'},
        {"elements", required_argument,     0,    'N'},
        {"universe", required_argument,     0,    'm'},
        {"count",    required_argument,     0,    'c'},
        {"file",     required_argument,     0,    'f'},
        {"seed1",    required_argument,     0,    '1'},
        {"seed2",    required_argument,     0,    '2'},
    };

	double alpha   = 0.5;
	uint64_t count = (1 << 25);
	uint32_t N     = (1 << 20);
	uint32_t m     = UINT32_MAX;
	char *filename = NULL;

	while( (opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1) {
		switch(opt) {
			case 'a':
				alpha = strtod(optarg, NULL);
				break;
			case 'N':
				N = strtol(optarg, NULL, 10);
				break;
			case 'm':
				m = strtol(optarg, NULL, 10);
				break;
			case 'c':
				count = strtoll(optarg, NULL, 10);
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
			case 'h':
			default:
				printusage(argv);
				exit(EXIT_FAILURE);
		}

		if ( errno != 0 ) {
			if (filename != NULL) {
				free(filename);
			}
			xerror(strerror(errno), __LINE__, __FILE__);
		}
	}

	map   = xmalloc( sizeof(uint32_t) * N );
	table = xmalloc( sizeof(double) * N );
	res   = xmalloc( sizeof(uint32_t) * BUFFER );
	file  = fopen(filename, "wb");
	if ( file == NULL ) {
		free(res);
		free(filename);
		free(table);
		xerror("Failed to open/create file.", __LINE__, __FILE__);
	}

	fprintf(file, "#N:        %"PRIu32"\n", N);
	fprintf(file, "#Universe: %"PRIu32"\n", m);
	fprintf(file, "#Alpha:    %lf\n", alpha);
	fprintf(file, "#Count:    %"PRIu64"\n", count);
	fprintf(file, "#Filename: %s\n", filename);
	fprintf(file, "#Seed1:    %"PRIu32"\n", I1);
	fprintf(file, "#Seed2:    %"PRIu32"\n", I2);

	for (i = 1; i <= N; i++) {
		table[i-1] = pow( (double)i, alpha );
		c = c + (1. / table[i-1]);
	}
	c = 1./c;

	for (i = 1; i <= N; i++) {
		map[i-1]   = xuni_rand()*m;
		table[i-1] = (c / table[i-1]);
	}

	// Print the weights of the top k probabilities into header of file
	fprintf(file, "#====== TOP %d ======\n", TOPK);
	for (i = 0; i < TOPK && i < N; i++) {
		fprintf(file, "#%"PRIu32": %lf\n", map[i], table[i]);
	}

	alias_t *a = alias_preprocess(N, table);

	i = 0;
	while ( likely(i < count) ) {
		memset(res, '\0', sizeof(uint32_t)*BUFFER);
		for (j = 0; likely( (i < count && j < BUFFER) ); j++, i++) {
			res[j] = map[(uint32_t)alias_draw(a)];
		}

		if ( unlikely(fwrite(res, sizeof(uint32_t), j, file) < j) ) {
			fclose(file);
			free(res);
			free(filename);
			free(table);
			xerror("Failed to write all data.", __LINE__, __FILE__);
		}
	}

	alias_free(a);
	fclose(file);
	free(res);
	free(filename);
	free(table);
	free(map);

	return EXIT_SUCCESS;
}
