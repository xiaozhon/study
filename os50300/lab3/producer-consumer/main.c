#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "producer_consumer.h"

/**
 * Initialization of thread information structure
 * 
 * @arguments 
 *		LV lv: thread information structure
 *		int tid: thread id
 *		GV gv: globle variables
 *		ring_buffer buf: ring buffer
 *		Sem s1: semaphore, empty Count
 *		Sem s2: semaphore, fullCount
 *		Sem s3: semaphore, mutex
**/
void init_thrd(TD thrd, int tid, GV gv, ring_buffer buf, Sem s1, Sem s2, Sem s3) {
	thrd -> tid = tid;
	thrd -> buf = buf;
	thrd -> emptyCount = s1;
	thrd -> fullCount = s2;
	thrd -> mutex = s3;
	thrd -> gv = gv;
}

/**
 * Initialization of globle variables
 * 
 * @arguments 
 *		GV gv: pointer to globle variables
 *		char* arg[]: variable values
**/
void init_gv(GV gv, char* arg[]) {
	int producerNum 		= atoi(arg[1]);
	int consumerNum 		= atoi(arg[2]);
	int ms 			= atoi(arg[3]);
	int total_number 	= atoi(arg[4]);
	
	gv -> producer_count = 0;
	gv -> consumer_count = 0;
	
	gv -> total_items_2_produce = total_number;
	gv -> max_sleep = ms;
	
	gv -> num_p_thrds = producerNum;
	gv -> num_c_thrds = consumerNum;
}

/**
 * Initialization of ring buffer
 * 
 * @arguments 
 *		ring_buffer: pointer to ring buffer
 *		char* arg[]: variable values
**/
void init_buf(ring_buffer buf, char *arg[]) {
	int buffer_size = atoi(arg[5]);
	
	buf -> size = buffer_size;
	buf -> head = 0;
	buf -> tail = 0;
	buf -> buffer = (int*) malloc(sizeof(int) * buffer_size);
}

/**
 * Initialization of semaphores
 * 
 * @arguments 
 *		Sem s: the semaphore
 *		int number: the initialization number
**/
void init_sem(Sem s, int number) {
	s -> S = number;
	pthread_mutex_init(&s->lock, NULL);
	pthread_cond_init(&s->cv, NULL);
}

/**
 * Main function, initialize all data structures, create producer and consumer threads
**/
int main(int argc, char* argv[]) {
	int i;
	int producerNum, consumerNum;
	int MAX_SLEEP_SECONDS;
	int total_number_items;
	int buffer_size;
	TD	 			p_thrds;		// pointer to all producer threads structure
	TD				c_thrds;		// pointer to all consumer threads structure
	
	pthread_t		*p_t;		// producer threads
	pthread_attr_t  *p_attrs;
	
	pthread_t		*c_t;		// consumer threads
	pthread_attr_t  *c_attrs;
	
	// shared memories
	GV				gv;
	ring_buffer		buf;
	Sem				emptyCount;
	Sem				mutex;
	Sem				fullCount;
	
	void            *retval;	// used in "join"
	
	if(argc != 6) {
		fprintf(stderr, "Usage: %s num_producer num_consumer ", argv[0]);
		fprintf(stderr, "max_sleep_seconds total_items buf_size \n");
		exit(1);
	}
	// get arguments
	producerNum 		= atoi(argv[1]);
	consumerNum 		= atoi(argv[2]);
	MAX_SLEEP_SECONDS	= atoi(argv[3]);
	total_number_items	= atoi(argv[4]);
	buffer_size 		= atoi(argv[5]);
	
	// validation of input argumants
	if(producerNum <= 0) {
		fprintf(stderr, "Producer Number should be greater than 0 \n");
		exit(1);
	}

	if(consumerNum <= 0) {
		fprintf(stderr, "Consumer Number should be greater than 0 \n");
		exit(1);
    }

	if(MAX_SLEEP_SECONDS <= 0) {
		fprintf(stderr, "Max sleep seconds should be greater than 0 \n");
		exit(1);
	}
	if(total_number_items < 0) {
		fprintf(stderr, "total number of items to be produced should be greater than 0 \n");
		exit(1);
	}
	
	if(buffer_size <= 0) {
		fprintf(stderr, "ring buffer size should be greater than 1 \n");
		exit(1);
	}

	// allocate memory space
	p_thrds = (TD)malloc(sizeof(*p_thrds)*producerNum);
	c_thrds = (TD) malloc(sizeof(*c_thrds) * consumerNum);
	
	// pointer to all thrds;
	p_t = (pthread_t*) malloc(sizeof(pthread_t)*producerNum);
	p_attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*producerNum);
	
	c_t = (pthread_t*) malloc(sizeof(pthread_t)*consumerNum);
	c_attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*consumerNum);
	
	// shared memory
	gv = (GV) malloc(sizeof(*gv));
	buf = (ring_buffer) malloc(sizeof(*buf));
	emptyCount = (Sem) malloc(sizeof(*emptyCount));
	fullCount = (Sem) malloc(sizeof(*fullCount));
	mutex = (Sem) malloc(sizeof(*mutex));
	// initialization of all shared data structure
	init_gv(gv, argv);
	init_buf(buf, argv);
	init_sem(emptyCount, buf -> size);
	init_sem(fullCount, 0);
	init_sem(mutex, 1);
		
	// create producer threads
	for (i = 0; i < producerNum; i++) {
		init_thrd(p_thrds + i, i, gv, buf, emptyCount, fullCount, mutex);		// producer threads initialization
		if(pthread_attr_init(p_attrs+i)) 	// initialize attributes
			perror("p_attr_init()");
		if(pthread_attr_setscope(p_attrs + i, PTHREAD_SCOPE_SYSTEM)) 	// set contention scope
			perror("p_attr_setscope()");
		if(pthread_create(p_t + i, p_attrs + i, do_producer, p_thrds + i)) {	// create threads
			perror("p_pthread_create()");
			exit(1);
		}
		printf("create producer thread %d \n", i);
	}

	// create consumer threads
	for (i = 0; i < consumerNum; i++) {
		init_thrd(c_thrds + i, i, gv, buf, emptyCount, fullCount, mutex);		// producer threads initialization
		if(pthread_attr_init(c_attrs+i)) 	// initialize attributes
			perror("c_attr_init()");
		if(pthread_attr_setscope(c_attrs + i, PTHREAD_SCOPE_SYSTEM)) 	// set contention scope
			perror("c_attr_setscope()");
		if(pthread_create(c_t + i, c_attrs + i, do_consumer, c_thrds + i)) {	// create threads
			perror("c_pthread_create()");
			exit(1);
		}
		printf("create consumer thread %d \n", i);
	}
	
	// join, wait threads to terminate
	for(i = 0; i < producerNum; i++) {
	//	printf("wait for producer: %d" ,i);
		pthread_join(p_t[i], &retval);
		printf("producer thread %d is finished\n", i);
	}
	
	for(i = 0; i < consumerNum; i++) {
		pthread_join(c_t[i], &retval);
		printf("consumer thread %d is finished\n", i);
	}
	
	printf("Main: consumed total: %d, produced total: %d \n", gv -> consumer_count, gv -> producer_count);
  
	// free memories
	free(p_thrds);
	free(c_thrds);
	free(p_t);
	free(c_t);
	free(p_attrs);
	free(c_attrs);
	free(buf -> buffer);
	free(buf);
	free(gv);
	free(emptyCount);
	free(fullCount);
	free(mutex);
	return 0; 

}

