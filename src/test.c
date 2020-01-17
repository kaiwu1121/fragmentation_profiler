

#include <stdio.h>
#include <stdlib.h>



#define TEST_NR	1000

int main(int argc, char **argv)
{
	void *ptrs[TEST_NR];
	int   sizes[TEST_NR];
	int i = 0;

	setbuf(stdout, NULL);

	printf("testing memory allocation and deallocation.\n");

	srand(time(NULL));

	for(i = 0; i < TEST_NR; i++) {
		sizes[i] = rand() % 40960;
		ptrs[i]  = malloc(sizes[i]);

		printf("alloc ptr = %lx size = %ld\n", ptrs[i], sizes[i]);

        if(i != 0 && i%10 == 0) {
            sleep(1);
        }
	}


	printf("wait for 60S to free ...\n");
	sleep(60);

	for(i = 0; i < TEST_NR; i++) {

        if(ptrs[i]) {
		    free(ptrs[i]);
        }

		printf("free ptr = %lx\n", ptrs[i]);

        if(i != 0 && i%10 == 0) {
            sleep(1);
        }

	}

	return 0;
}
