#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "htrace_lib.h"



void *htrace_tmp_alloc(size_t size)
{
	void *ptr;
	void *rptr;
	
	struct htrace_memory_header *header;


	size_t rsize = size + sizeof(*header);

    //printf("size=%d\n", size);

	ptr = mmap(0, rsize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);

  //  printf("ptr=%p\n", ptr);
	
    if(ptr) {
		header = (struct htrace_memory_header *)ptr;
		header->magic = HTRACE_TMP_MAGIC;
		header->size  = size;
		header->ptr	  = ptr;
	}

	rptr = (void*)(ptr + sizeof(*header));

    //exit(0);
	return rptr;
}

void htrace_tmp_free(void *ptr)
{
	struct htrace_memory_header *header;

	header = (struct htrace_memory_header *)(ptr - sizeof(*header));

	if(header->magic != HTRACE_TMP_MAGIC) {
		return ;
	}

	if(header->size == 0) {
		return;
	}

	if(header->ptr == NULL) {
		return;
	}


	void *rptr = ptr - sizeof(*header);

	if(rptr != header->ptr) {
		return;
	}

	(void) munmap(rptr, header->size);
}
