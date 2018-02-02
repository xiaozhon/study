#include <pthread.h>

/**
 * Data structure:
 *		thread information
 *		ring buffer
 *		semaphore: emptyCount
 *		semaphore: mutex
 * 		semaphore: fullCount
 *		shared variables: counters, max_sleep, total_items_2_produce
**/
typedef struct thrd_t {
	int	tid;
	void *buf;
	void *emptyCount;
	void *fullCount;
	void *mutex;
	void *gv;	// shared memory
}* TD;

typedef struct ring_buffer_t {
	int size;
	int* buffer;
	int head;
	int tail;
}* ring_buffer;

typedef struct sem_t {
	int S;
	pthread_mutex_t lock;
	pthread_cond_t  cv;
}* Sem;

typedef struct gv_t {
	int producer_count;	// the number of produced items
	int consumer_count;	// the number of consumed items
	
	int total_items_2_produce;		// max number of items to be produced
	int max_sleep;
	int num_p_thrds;		// the number of producer threads
	int num_c_thrds;		// the number of consumer threads
}* GV;

void* do_producer(void* arg);
void* do_consumer(void* arg);
void P(Sem s);
void V(Sem s);


