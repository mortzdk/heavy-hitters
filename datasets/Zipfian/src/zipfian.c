#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <inttypes.h>
#include <errno.h>

#include "alias.h"
#include "xutil.h"

#define BUFFER 1024*512
#define TOPK 10

static double *table;
static double c = 0;

static void printusage(char *argv[]) {
    fprintf(stderr, "Usage: %s \n"
            "\t[-f --file     [char *]   {REQUIRED} (Filename to write to)]\n"
            "\t[-a --alpha    [double]   {OPTIONAL} (Alpha value)]\n"
            "\t[-c --count    [uint64_t] {OPTIONAL} (Times to sample)]\n"
            "\t[-N --universe [uint64_t] {OPTIONAL} (Amount of unique items)]\n"
            "\t[-1 --seed1    [uint32_t] {OPTIONAL} (First seed value)]\n"
            "\t[-2 --seed2    [uint32_t] {OPTIONAL} (Second seed value)]\n"
            "\t[-h --help                {OPTIONAL} (Shows this guideline)]\n"
            , argv[0]);
}

int main (int argc, char**argv) {
	FILE *file;
	uint64_t j, i = 0;
	uint64_t *res;

	/* getopt */
	int opt;
    int option_index = 0;
    static const char *optstring = "1:2:c:N:a:f:h";
    static const struct option long_options[] = {
        {"alpha",    required_argument,     0,    'a'},
        {"universe", required_argument,     0,    'N'},
        {"count",    required_argument,     0,    'c'},
        {"file",     required_argument,     0,    'f'},
        {"seed1",    required_argument,     0,    '1'},
        {"seed2",    required_argument,     0,    '2'},
    };

	double alpha   = 0.5;
	uint64_t count = (1 << 25);
	uint64_t N     = (1 << 20);
	char *filename = NULL;

	while( (opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1) {
		switch(opt) {
			case 'a':
				alpha = strtod(optarg, NULL);
				break;
			case 'N':
				N = strtoll(optarg, NULL, 10);
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


	printf("===========\n");
	printf("Parameters:\n");
	printf("===========\n");
	printf("N:        %"PRIu64"\n", N);
	printf("Alpha:    %lf\n", alpha);
	printf("Count:    %"PRIu64"\n", count);
	printf("Filename: %s\n", filename);
	printf("Seed1:    %"PRIu32"\n", I1);
	printf("Seed2:    %"PRIu32"\n", I2);
	printf("===========\n\n");

	table = xmalloc( sizeof(double) * N );
	res   = xmalloc( sizeof(uint64_t) * BUFFER );
	file  = fopen(filename, "wb");
	if ( file == NULL ) {
		free(res);
		free(filename);
		free(table);
		xerror("Failed to open/create file.", __LINE__, __FILE__);
	}

	for (i = 1; i <= N; i++) {
		table[i-1] = pow( (double)i, alpha );
		c = c + (1. / table[i-1]);
	}
	c = 1./c;

	for (i = 1; i <= N; i++) {
		table[i-1] = (c / table[i-1]);
	}

	int cnt[TOPK];
	memset(cnt, '\0', sizeof(int)*TOPK);

	alias_t * a = alias_preprocess(N, table);

	i = 0;
	while ( likely(i < count) ) {
		for (j = 0; likely( (i < count && j < BUFFER) ); j++, i++) {
			res[j] = alias_draw(a);
			if (res[j] < TOPK) {
				cnt[res[j]] += 1;
			}
		}

		if ( unlikely(fwrite(res, sizeof(uint64_t), j, file) < j) ) {
			fclose(file);
			free(res);
			free(filename);
			free(table);
			xerror("Failed to write all data.", __LINE__, __FILE__);
		}
	}

	printf("====== TOP %d ======\n", TOPK);
	for (i = 0; i < TOPK; i++) {
		printf("%"PRIu64": %lf\n", i+1, (double)cnt[i]/count);
	}

	alias_free(a);
	fclose(file);
	free(res);
	free(filename);
	free(table);

	return EXIT_SUCCESS;
}
