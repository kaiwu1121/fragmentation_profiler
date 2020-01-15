

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <dlfcn.h>

#include <execinfo.h>


#include <fcntl.h>
#include <string.h>

#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>


#include "spinlock.h"

#include "htrace_lib.h"


int main(int argc, char **argv)
{
	struct htrace_record record;
	int fd = -1;
	size_t num = 1;
	FILE *fp = stdout;

	setbuf(stdout, NULL);

	if(argc != 2 && argc != 3) {
		fprintf(stderr, "%s <trace-data-file> [output-file.txt]\n", argv[0]);
		exit(-1);
	}

	if(argc == 3) {
		fp = fopen(argv[2], "w+");

		if(fp == NULL)
			fp = stdout;
	}

	fd = open(argv[1], O_RDONLY, 0666);

	if(fd < 0) {
		fprintf(stderr, "failed open file %s\n", argv[1]);
		exit(-1);
	}
	
	while(read(fd, &record, sizeof(record)) == sizeof(record)) {
		
	//	if(record.magic != HTRACE_RECORD_MAGIC) {
	//		fprintf(stderr, "Invalid record\n");
	//		continue;
	//	}

		fprintf(fp, "id=%d ptr=%p size=%ld\n", record.id, record.ptr, record.size);

		num++;
	}

	close(fd);

	if(fp != stdout) {
		fclose(fp);
		printf("Total %d records, %s -> %s is converted.\n", num - 1, argv[1], argv[2]);
	}

	return 0;
}
