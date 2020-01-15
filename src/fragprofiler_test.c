

#include <stdio.h>
#include <stdlib.h>

#include "htrace_lib.h"


#define TEST_NR	100

int main(int argc, char **argv)
{
	void *ptrs[TEST_NR];
	int   sizes[TEST_NR];
	int i = 0;

	setbuf(stdout, NULL);

	printf("testing memory allocation and deallocation.\n");

	srand(time(NULL));

	for(i = 0; i < TEST_NR; i++) {
		sizes[i] = rand() % 4096;
		//ptrs[i]  = htrace_tmp_alloc(sizes[i]);
		ptrs[i]  = malloc(sizes[i]);

		printf("alloc ptr = %lx size = %ld\n", ptrs[i], sizes[i]);
	}


	printf("wait for 60S ...\n");

	sleep(60);

	for(i = 0; i < TEST_NR; i++) {
		//htrace_tmp_free(ptrs[i]);
		free(ptrs[i]);

		printf("free ptr = %lx\n", ptrs[i]);
	}




	return 0;
}
