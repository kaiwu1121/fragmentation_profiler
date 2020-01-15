/*
 *
 * Author: Jiaolin Luo <jluo38@ucmerced.edu>
 * 
 *
 * */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "spinlock.h"
#include "fprof_lib.h"


#define FPROF_ADDR_HASH_SIZE    10000

//struct rb_root fprof_lyt_root = RB_ROOT;
//static size_t	fprof_lyt_pages = 0;

static FILE *fprof_dump_fp = NULL;
static char fprof_dump_file[128];

static spinlock_t fprof_init_lock = SPIN_UNLOCKED;

//static spinlock_t fprof_lyt_lock = SPIN_UNLOCKED;

static int fprof_init_flag = 0;


static size_t       fprof_objects_size_bytes = 0; //in bytes
static spinlock_t   fprof_objects_size_lock = SPIN_UNLOCKED;

static pthread_t    fprof_dump_thread;
static int          fprof_dump_runflag = 0;

static void* (*real_calloc)(size_t, size_t) = NULL;
static void* (*real_malloc)(size_t) = NULL;
static void* (*real_realloc)(void *, size_t) = NULL;

static void (*real_free)(void*) = NULL;

static long fprof_time_id = 0;


#if 1
static int insert_record(struct rb_root *root, struct fprof_node *data)
{
  	struct rb_node **new = &(root->rb_node), *parent = NULL;

  	/* Figure out where to put new node */
  	while (*new) {
  		struct fprof_node *this = container_of(*new, struct fprof_node, node);

  		//int result = strcmp(data->string, this->string);
  		long result = (unsigned long)data->ptr - (unsigned long)this->ptr;

		parent = *new;

  		if (result < 0)
  			new = &((*new)->rb_left);
  		else if (result > 0)
  			new = &((*new)->rb_right);
  		else
  			return 0;
  	}

  	/* Add new node and rebalance tree. */
  	rb_link_node(&data->node, parent, new);
  	rb_insert_color(&data->node, root);

	return 1;
}

static struct fprof_node * find_record(struct rb_root *root, void *ptr)
{
  	struct rb_node *node = root->rb_node;

  	while (node) {
  		struct fprof_node *data = container_of(node, struct fprof_node, node);

		long result;

		//result = strcmp(string, data->string);
		result = (unsigned long)ptr - (unsigned long)data->ptr;

		if (result < 0)
  			node = node->rb_left;
		else if (result > 0)
  			node = node->rb_right;
		else
  			return data;
	}

	return NULL;
}

static size_t delete_record(struct rb_root *root, void *ptr)
{
	struct fprof_node *node = find_record(root, ptr);
	size_t size = 0;

	if(node) {
		size = node->size;
		rb_erase(&node->node, root);

		fprof_tmp_free(node);
	}

	return size;
}
#endif

void free(void *ptr)
{

	//struct fprof_memory_header *hdr = (struct fprof_memory_header *) (ptr - sizeof(*hdr));
	size_t size = 0;;

	
	//if(ptr) {
	//	if(hdr->magic == HTRACE_TMP_MAGIC && hdr->ptr == ptr)
	//		size = hdr->size;
	//} 

	if(real_free)
		real_free(ptr);

	spin_lock(&fprof_lyt_lock);
	size = delete_record(&fprof_lyt_root, ptr);
	fprof_lyt_pages--;
	spin_unlock(&fprof_lyt_lock);

	spin_lock(&fprof_objects_size_lock);
	fprof_objects_size_bytes -= size;
	spin_unlock(&fprof_objects_size_lock);
}

void* malloc(size_t size) 
{
	void *ptr = NULL;
	int ret = 0;
	
	struct fprof_node *node = NULL; 

	node = fprof_tmp_alloc(sizeof(*node));
	
	if(node == NULL) {
		return NULL;
	}
#if 0
	struct fprof_memory_header *hdr = NULL;
	size_t _size = size + sizeof(*hdr);

	if(real_malloc) {
		hdr = real_malloc(_size);
		
		if(hdr == NULL)
			return NULL;

		ptr = hdr + sizeof(*hdr);

		hdr->magic = HTRACE_TMP_MAGIC;
		hdr->size = size;
		hdr->ptr = ptr;

	}
#else
	if(real_malloc) {
		ptr = real_malloc(size);
	}
#endif

	if(ptr == NULL) {
		fprof_tmp_free(node);
		return NULL;
	}

	node->ptr  = ptr;
	node->size = size;

	spin_lock(&fprof_lyt_lock);
	insert_record(&fprof_lyt_root, node);
	fprof_lyt_pages++;
	spin_unlock(&fprof_lyt_lock);	


	spin_lock(&fprof_objects_size_lock);
	fprof_objects_size_bytes += size;
	spin_unlock(&fprof_objects_size_lock);

	return ptr;
}

#if 0
void* calloc(size_t count, size_t size)
{
	void *ptr = NULL;
	struct fprof_memory_header *hdr = NULL;
	size_t _size = size + sizeof(*hdr);

	//struct fprof_node *node = fprof_tmp_alloc(sizeof(*node));
	
	//if(node == NULL) {
	//	return NULL;
	//}


	//fprof_wait_init_done();

	if(real_calloc) {
		ptr = real_calloc(count, size);
	}

	if(ptr == NULL) {
		fprof_tmp_free(node);
		return NULL;
	}


	node->ptr = ptr;
	node->size = count * size;

	spin_lock(&fprof_lyt_lock);
	insert_record(&fprof_lyt_root, node);
	spin_unlock(&fprof_lyt_lock);

	spin_lock(&fprof_objects_size_lock);
	fprof_objects_size_bytes += count * size;
	spin_unlock(&fprof_objects_size_lock);

	return ptr;
}

void *realloc(void *ptr, size_t size)
{
	void *new_ptr;
	struct fprof_node *node = fprof_tmp_alloc(sizeof(*node));
	size_t oldsize = 0;

	if(node == NULL) {
		return NULL;
	}

	if(real_realloc)
		new_ptr = real_realloc(ptr, size);

	if(new_ptr == NULL) {
		fprof_tmp_free(node);
		return NULL;
	}


	node->ptr = new_ptr;
	node->size = size;

	spin_lock(&fprof_lyt_lock);
	
	//delete old
	oldsize = delete_record(&fprof_lyt_root, ptr);

	//insert new
	insert_record(&fprof_lyt_root, node);
	
	spin_unlock(&fprof_lyt_lock);

	spin_lock(&fprof_objects_size_lock);
	fprof_objects_size_bytes -= oldsize;
	fprof_objects_size_bytes += size;
	spin_unlock(&fprof_objects_size_lock);

	return new_ptr;
}
#endif

static void libfprof_record_write(void *ptr, size_t sz)
{
	struct fprof_record record;


	if(fprof_dump_fp == NULL)
		return;

	memset(&record, 0, sizeof(record));

	//gettimeofday(&record.tv, NULL);

	record.id	 = fprof_time_id;
	//record.magic = HTRACE_RECORD_MAGIC;
	record.ptr   = (unsigned long)ptr;
	record.size  = sz;

	fwrite(&record, sizeof(record), 1, fprof_dump_fp);
}

#define FPROF_SET_REAL_FUNC(real_ptr, name) do {\
	real_ptr = dlsym(RTLD_NEXT, name); \
	\
	if(real_ptr == NULL) {\
		/*fprintf(stderr, "can't find symbol %s\n", name);*/\
		exit(-1);\
	}\
}while(0)


static long fprof_tv_diff_secs(struct timeval *tv1, struct timeval *tv2)
{
	long ret = 0;


	if(tv2->tv_sec > tv1->tv_sec)
		ret = (tv2->tv_sec - tv1->tv_sec);
	else
		ret = 0;

	if(tv2->tv_usec < tv1->tv_usec)
		ret -= 1;

#if 0
	ret += tv2->tv_usec - tv1->tv_usec;
#endif

	if(ret < 0)
		ret = 0;

	return ret;
}


#if 0
#define HTRACE_PERF_PATH	"/usr/bin/perf"



struct fprof_stats {
	size_t tlbmisses;
	size_t dcache_loads;
	size_t dcache_misses;
	size_t pagefaults;
	size_t vmsize;
	size_t vmrss;
	size_t rssratio;
};

static void collect_sys_stats(void)
{	

	pid_t 	child_pid;
	int 	child_status = -1;
	char	*argv[128];
	int		argc = 0;
	int		child_ret;
	
	pid_t pid = getpid();
	char	_pid[32];

	memset(_pid, 0, sizeof(_pid));

	snprintf(_pid, sizeof(_pid), "%d", pid);

	//
	// perf stat  -e page-faults,L1-dcache-loads,L1-dcache-load-misses sleep 2
	//
	// --repeat 10
	// --log-fd 
	// --output file
	//perf stat -d -d  --pid 63539 --interval-print 1000 --event dTLB-load-misses,iTLB-load-misses
	// page-faults
	// major-faults,minor-faults,
	// L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,
	// L1-icache-load-misses,
	// LLC-load-misses,LLC-loads,LLC-store-misses,LLC-stores,
	// branch-load-misses,branch-loads,
	// dTLB-load-misses,dTLB-loads,dTLB-store-misses,dTLB-stores,
	// iTLB-load-misses,iTLB-loads,
	//

	// PMU
	// branch-instructions,
	// branch-misses,
	// cache-misses
	//
	// power/energy-cores/ 
	// power/energy-pkg/  
	// power/energy-ram/ 
	//

	/*

	
          0.672663      task-clock (msec)         #    0.001 CPUs utilized          
                 1      context-switches          #    0.001 M/sec                  
                 0      cpu-migrations            #    0.000 K/sec                  
                53      page-faults               #    0.079 M/sec                  
           803,668      cycles                    #    1.195 GHz                    
   <not supported>      stalled-cycles-frontend  
   <not supported>      stalled-cycles-backend   
           616,464      instructions              #    0.77  insns per cycle        
           127,680      branches                  #  189.813 M/sec                  
             6,544      branch-misses             #    5.13% of all branches        

	*/

	argv[0] = HTRACE_PERF_PATH;
	argv[1] = "-p";
	argv[2] = _pid;
	argv[3] = "-e";
	argv[4] = "page-faults,";
	argv[5] = "sleep";
	argv[6] = "1";
	argv[7] = NULL;

	argc = 8;

	child_pid = fork();

	if(child_pid == 0) {
		//in child

		child_status = execvp(argv[0], argv);

		exit(child_status);
	}

//	while(child_quit == 0)
//		sleep(1);

	waitpid(child_pid, &child_ret, 0);

}
#endif

static void dump_vmstat(void)
{
	FILE *fp = NULL;

	char procfile[32];

	//
	// filesystems/proc.txt line 1967
	//
	// /proc/self/status
    //

	memset(procfile, 0, sizeof(procfile));

	snprintf(procfile, sizeof(procfile), "/proc/%d/status", getpid());

	fp = fopen(procfile, "r");

	if(fp == NULL) {
		fprintf(stderr, "FPROF: can't open %s", procfile);
		return ;
	}
	


#if 0
	//int len = fread(buf, sizeof(buf), 1, fp);

	size_t vmsize = 0; //in KB, exe+lib+data+stack
						//total program size (pages)  

	size_t vmrss = 0;  //resident size
						//size of memory portions (pages) 

	size_t shared=0;    //RssFile+RssShmem
						// number of pages that are shared   (i.e. backed by a file, same
                        //  as RssFile+RssShmem in status)


	size_t trs = 0;    	//code
						//number of pages that are 'code'   (not including libs; broken,
						//includes data segment)

	size_t lrs = 0;     // number of pages of library        (always 0 on 2.6)

	size_t drs = 0;     // number of pages of data/stack     (including libs; broken, includes library text)

	size_t dt= 0;       //number of dirty pages         (always 0 on 2.6)

	//
	// eg 21233 518 358 2 0 18654 0
	//
	fscanf(fp, "%lu %lu %lu %lu %lu %lu %lu", &vmsize, &vmrss, &shared, &trs, &lrs, &drs, &dt);
#endif

	/*
Name:	xxx
State:	S (sleeping)
Tgid:	75655
Ngid:	0
Pid:	75655
PPid:	75594
TracerPid:	0
Uid:	1000	1000	1000	1000
Gid:	1000	1000	1000	1000
FDSize:	64
Groups:	4 20 24 25 27 29 30 44 46 109 1000 
NStgid:	75655
NSpid:	75655
NSpgid:	75594
NSsid:	75594
VmPeak:	   94640 kB
VmSize:	   94640 kB
VmLck:	       0 kB
VmPin:	       0 kB
VmHWM:	    5100 kB
VmRSS:	    5100 kB
VmData:	    1948 kB
VmStk:	     132 kB
VmExe:	     756 kB
VmLib:	    8744 kB
VmPTE:	     196 kB
VmPMD:	      16 kB
VmSwap:	       0 kB
HugetlbPages:	       0 kB
Threads:	1
SigQ:	0/514617
SigPnd:	0000000000000000
ShdPnd:	0000000000000000
SigBlk:	0000000000000000
SigIgn:	0000000000001000
SigCgt:	0000000180010000
CapInh:	0000000000000000
CapPrm:	0000000000000000
CapEff:	0000000000000000
CapBnd:	0000003fffffffff
CapAmb:	0000000000000000
Seccomp:	0
Speculation_Store_Bypass:	thread vulnerable
Cpus_allowed:	ffffffff,ffffffff,ffffffff,ffffffff,ffffffff,ffffffff
Cpus_allowed_list:	0-191
Mems_allowed:	00000000,00000003
Mems_allowed_list:	0-1
voluntary_ctxt_switches:	16451
nonvoluntary_ctxt_switches:	0


	*/

	size_t vmrss = 0;
	size_t vmpte = 0;
	size_t vmsize = 0;

	int count = 0;

	char *k = NULL;
	char *v = NULL;

	int ret = -1;

	char *line = NULL;
	size_t len = 0;

	while(!feof(fp)) {


		line = NULL;
		len  = 0;

		ret = getline(&line, &len, fp);

		if(ret <= 0)
			break;

		//printf("%s\n", line);


		v = line;

		k = strsep(&v, ":");

		if(k == NULL) {
			free(line);
			continue;
		}

		//skip blank

		while(*v == '\t' || *v == ' ')
			v++;

		char *tmp = v;

		v = strsep(&tmp, " ");

		//printf("k=%s,v=%s\n", k, v);

		if(strcmp(k, "VmRSS") == 0) {
			//printf("found VmRSS: %s\n", v);
			vmrss = atol(v);
			//sscanf(v, "%lu", &vmrss);
			count++;
		}

		if(strcmp(k, "VmPTE") == 0) {
			//printf("found VmPTE: %s\n", v);
			vmpte = atol(v);
			//sscanf(v, "%lu", &vmpte);
			count++;
		}

		if(strcmp(k, "VmSize") == 0) {
			//printf("found VmSize: %s\n", v);
			vmsize = atol(v);
			//sscanf(v, "%lu", &vmsize);
			count++;
		}


		free(line);

		if(count == 3) 
			break;



	}


	fclose(fp);

	// /proc/stat
	//cpu  3394538 19563 850758 17010260449 402256 0 81080 0 0 0




	double rss_ratio = 0;

	size_t tmppages = 0;

	//
	//we must exclude the pages we allocated in our tmp_alloc function.
	//
	spin_lock(&fprof_lyt_lock);
	tmppages = fprof_lyt_pages;
	spin_unlock(&fprof_lyt_lock);

	vmrss -= tmppages * 4; //4K

	if(vmrss < 0)
		vmrss = fprof_objects_size_bytes / 1024;

	rss_ratio = (fprof_objects_size_bytes + 0.01) / (vmrss * 1024 + 0.01);

	fprintf(	fprof_dump_fp, 
				"time=%d,vmsize=%lu,vmrss=%lu,vmpte=%lu,objects=%lu,rss_ratio=%.3f\n", 
				fprof_time_id,
				vmsize,
				vmrss,
				vmpte,
				fprof_objects_size_bytes >> 10,
				rss_ratio
                );

    char *_fprof_debug = getenv("fprof_debug");

    if(_fprof_debug == NULL) {
        _fprof_debug = "0";
    }

    int fprof_debug = atoi(_fprof_debug);

    if(fprof_debug) {
        printf("vmsize=%lu,vmrss=%lu,rss_ratio=%.3f\n", vmsize, vmrss, rss_ratio);
    }
}

static void *fprof_dump_thread_func(void *arg)
{
	struct timeval tv0, tv1, tv2;
	long t1 = 0;
	long t2 = 0;
	struct rb_node *node;
	struct fprof_node *entry;
	int ret = -1;
	struct stat st;
	int n = 0;

	fprof_dump_runflag = 1;

	(void) gettimeofday(&tv0, NULL);
	(void) gettimeofday(&tv1, NULL);
	(void) gettimeofday(&tv2, NULL);

    char *_fprof_max_runs = getenv("fprof_max_runs");

    if(_fprof_max_runs == NULL) {
        _fprof_max_runs = "-1";
    }

    int fprof_max_runs = atoi(_fprof_max_runs);

    if(fprof_max_runs == 0) {
        fprof_max_runs = -1;
    }

    char *_fprof_max_size = getenv("fprof_max_size");

    if(_fprof_max_size == NULL) {
        _fprof_max_size = "-1";
    }

    size_t fprof_max_size = atoll(_fprof_max_size);

    if(fprof_max_size == 0) {
        fprof_max_size = -1;
    }

    //MB to bytes
    fprof_max_size = fprof_max_size << 20;

    char *_fprof_dump_interval = getenv("fprof_dump_interval");

    if(_fprof_dump_interval == NULL) {
        _fprof_dump_interval = "1";
    }

    int fprof_dump_interval = atoll(_fprof_dump_interval);


    printf("FPROF: fprof_max_runs=%d\n", fprof_max_runs);
    printf("FPROF: fprof_max_size=%lld\n", fprof_max_size);
    printf("FPROF: fprof_dump_interval=%d\n", fprof_dump_interval);


	while(fprof_dump_runflag == 1) {

		(void) gettimeofday(&tv2, NULL);

		t1 = fprof_tv_diff_secs(&tv0, &tv2);

		t2 = fprof_tv_diff_secs(&tv1, &tv2);

		if(fprof_max_runs > 0 && n >  fprof_max_runs) {
			fclose(fprof_dump_fp);
            printf("FPROF: exited.\n");
			exit(0);
		}

		if(t2 < fprof_dump_interval) {
			sleep(1);
			continue;
		}

		n++;

		(void) gettimeofday(&tv1, NULL);
		(void) gettimeofday(&tv2, NULL);


		dump_vmstat();

        //
		// check size
        //
		ret = stat(&fprof_dump_file, &st);

		fprof_time_id++;

		if(ret == 0 && (st.st_size >> 20) < fprof_max_size) {
			continue;
		} else {
			fclose(fprof_dump_fp);
            printf("FPROF: dump file size %lld MB exceeled the limit %lld MB\n", st.st_size >> 20, fprof_max_size >> 20);
			exit(0);
		}


	}

	return NULL;
}

void __attribute__((constructor)) libfprof_init(void)
{
	int i = 0;
	int ret = -1;
	struct stat st;
	
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	spin_lock(&fprof_init_lock);

	if(fprof_init_flag == 1) {
		spin_unlock(&fprof_init_lock);
		return;
	}

	//get real funcs
	
	FPROF_SET_REAL_FUNC(real_malloc,  "malloc");
	FPROF_SET_REAL_FUNC(real_calloc,  "calloc");
	FPROF_SET_REAL_FUNC(real_realloc, "realloc");
	FPROF_SET_REAL_FUNC(real_free,    "free");



	(void) mkdir(FPROF_RESULT_DIR, 0755);

	//create file
	for(i = 1; i <= 1024; i++) {
		snprintf(fprof_dump_file, sizeof(fprof_dump_file), "%s/trace_%d.txt", FPROF_RESULT_DIR, i);

		ret = stat(fprof_dump_file, &st);

		if(ret < 0)
			break;
	}

	fprof_dump_fp = fopen(fprof_dump_file, "w+t");

	if(fprof_dump_fp == NULL) {
		fprintf(stderr, "FPROF: failed to create trace file: %s\n", fprof_dump_file);
		spin_unlock(&fprof_init_lock);
		exit(-1);
	}

	setbuf(fprof_dump_fp, NULL);

	fprof_dump_runflag = 0;

	mfence();

	//create thread
	ret = pthread_create(&fprof_dump_thread, NULL, fprof_dump_thread_func, NULL);

	if(ret < 0) {
		fprintf(stderr, "FPROF: failed pthread_create\n");
		exit(-1);
	}

	mfence();

	while(fprof_dump_runflag == 0) {
		usleep(100 * 1000);
	}

	fprof_init_flag = 1;

	spin_unlock(&fprof_init_lock);
}

void __attribute__((destructor))  libfprof_exit(void)
{

	fprof_dump_runflag = 1;

	if(fprof_dump_fp >= 0) {
		fclose(fprof_dump_fp);
        fprof_dump_fp = -1;
	}
}


