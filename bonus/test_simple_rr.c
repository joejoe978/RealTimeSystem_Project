#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <sched.h>
#include <getopt.h>
#include <ctype.h>
#include <limits.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#define SCHED_NORMAL 0
#define SCHED_SIMPLE_RR 6

#define SYS_simple_rr_getquantum 337
#define SYS_simple_rr_setquantum 338

#define START_CHAR 97

struct thread_args {
  int tid;
  int prio;
  int nchars;
  char mychar;
};

int sched_policy, quantum, old_quantum, num_threads, buffer_size;
int total_num_chars;
char *val_buf;
int val_buf_pos = 0;
pthread_t *threads;

void fail(char* msg)
{
	printf("%s\n", msg);
	exit(-1);
}

void *run(void *arg) 
{
	int i;
	struct thread_args *my_args = (struct thread_args*) arg;
	
	//+ write characters to the val_bufs
	for(i = 0; i<my_args->nchars; i++)
	{
		if (val_buf_pos > total_num_chars)
			break;

		*(val_buf + val_buf_pos) = my_args->mychar;
		__sync_fetch_and_add(&val_buf_pos,1);
	}

	free(my_args);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	struct sched_param param;
	struct thread_args *targs;
	pthread_attr_t attr;
	int i;
	char cur;
	
	if( argc != 5 )
		fail("Invalid arguments count");
	
	//+ parse arguments
	if( strcmp(argv[1], "default") == 0 )
		sched_policy = SCHED_NORMAL;
	else if( strcmp(argv[1], "simple_rr") == 0 )
		sched_policy = SCHED_SIMPLE_RR;
	else
		fail("Invalid scheduling policy");
	
	quantum = atoi(argv[2]);
	num_threads = atoi(argv[3]);
	buffer_size = atoi(argv[4]);

	printf("sched_policy: %d, quantum: %d, num_threads: %d, buffer_size: %d\n", sched_policy, quantum, num_threads, buffer_size);

	int myprio[10], tmp;
	for (i = 0; i < num_threads; i++) {
		tmp = scanf("%d", &myprio[i]);
	}
	for (i = 0; i < num_threads; i++) {
		printf("%d", myprio[i]);
	}
	printf("\n");


	//+ set simple_rr scheduling policy
	if (sched_policy == SCHED_SIMPLE_RR)
	{
		param.sched_priority = 0;
		if ( sched_setscheduler(getpid(), sched_policy, &param) == -1)
		{
			perror("sched_setscheduler");
			fail("sched_setscheduler fail");
		};

		old_quantum = syscall (SYS_simple_rr_getquantum);
		syscall (SYS_simple_rr_setquantum, quantum);
    }
	
	//+ create the buffer
	if ( (val_buf = (char *) malloc(buffer_size)) == NULL )
		fail("malloc(buffer_size) fail");
	total_num_chars  = (buffer_size / sizeof(char));

	//+ create and start each thread
	if ( (threads = malloc(num_threads*sizeof(pthread_t))) == NULL )
		fail("malloc(num_threads) fail");
		
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	for (i = 0; i < num_threads; i++)
	{
		targs = malloc(sizeof(*targs));
		targs->tid    = i;
		//targs->prio   = i;
		targs->prio   = myprio[i];
		targs->mychar = (char) (i+START_CHAR);
		targs->nchars = (total_num_chars / num_threads);
		
		//++
		//syscall (SYS_simple_rr_setquantum, (num_threads - targs->prio) * quantum);
		if (targs->prio == 1) syscall (SYS_simple_rr_setquantum, 500 * quantum);
		if (targs->prio == 2) syscall (SYS_simple_rr_setquantum, 400 * quantum);
		if (targs->prio == 3) syscall (SYS_simple_rr_setquantum, 3 * quantum);
		if (targs->prio == 4) syscall (SYS_simple_rr_setquantum, 2 * quantum);
		if (targs->prio == 5) syscall (SYS_simple_rr_setquantum, 1 * quantum);


		pthread_create(&threads[i], &attr, run, (void *)targs);

		//printf("quantum:%d\n", syscall (SYS_simple_rr_getquantum));
	}

	//+ wait for all threads to complete
	for (i = 0; i < num_threads; i++) 
	{
		pthread_join(threads[i], NULL);
	}

	//+ print val_buf results
	for (i = 0; i < total_num_chars; i++) 
	{
		if (cur != val_buf[i]) 
		{
			cur = val_buf[i];
			printf("%c", cur);;
		}
	}
	printf("\n");
	
	//+ reset time quantum
	syscall (SYS_simple_rr_setquantum, old_quantum);

	//+ clean up and exit
	pthread_attr_destroy(&attr);
	pthread_exit (NULL);
}
