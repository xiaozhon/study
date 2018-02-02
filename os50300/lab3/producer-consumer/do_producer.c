#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "producer_consumer.h"

/**
 * Semaphore operation: P
 * 
 * @arguments 
 *		Sem s: the semaphore
**/
void P(Sem s) {
	pthread_mutex_lock(&s -> lock);	// we need this lock?
	
	while (s -> S <= 0) {
		pthread_cond_wait(&s -> cv, &s -> lock);
	}
	s -> S--;

	pthread_mutex_unlock(&s -> lock);
}

/**
 * Semaphore operation: P
 * 
 * @arguments 
 *		Sem s: the semaphore
**/
void V(Sem s) {
	pthread_mutex_lock(&s -> lock);	// we need this lock?
	s -> S++;
	pthread_mutex_unlock(&s -> lock);
	pthread_cond_signal(&s -> cv);
}

/**
 * Producer thread routine
**/
void* do_producer(void *v) {
	TD thrd;
	GV gv;
	Sem emptyCount;
	Sem fullCount;
	Sem mutex;
	ring_buffer buf;
	
	int sleep_t = 1;
	int i;
	
	// pointers to shared memory
	thrd = (TD) v;
	gv = (GV) thrd -> gv;
	buf = (ring_buffer) thrd -> buf;
	emptyCount = (Sem) thrd -> emptyCount;
	fullCount = (Sem) thrd -> fullCount;
	mutex = (Sem) thrd -> mutex;
	// while loop, where real work is done
	while(1) {
		// randomly sleep, produce item
		sleep_t = (random() % gv -> max_sleep) + 1;	
		sleep(sleep_t);

		P(emptyCount);
		// lock the buffer
		P(mutex);
		// put item in the buffer
		// only produce new item when the number doesn't reach the max
		if(gv -> producer_count < gv -> total_items_2_produce) {
			// if buffer full, do nothing
            // if we use "while" here, one producer may get the mutex and do busy waiting forever,
            // because no consumer can get into this section
            // if buffer size is 1, this is not a ring buffer
            if (buf -> size == 1){
                // use buf->head to indicate whether the item has been consumed
                // if buf->head = 0, the item has been consumed, otherwise it is still there
                // producer should do nothing
                if(buf -> head == 0){
                    buf -> buffer[buf -> head] = sleep_t;
                    buf -> head = 1;
                    gv -> producer_count++;
                }
            } else { // it is ring buffer
				// if buffer full, do nothing
				// if we use "while" here, one producer may get the mutex and do busy waiting forever,
				// because no consumer can get into this section
				if ((buf -> head + 1) % buf -> size == buf -> tail) {
					printf("P: buffer full, producer %d do nothing. \n \n", thrd -> tid);
				} else {
					buf -> buffer[buf -> head] = sleep_t;
					buf -> head = (buf -> head + 1) % buf -> size;
					gv -> producer_count++;
					printf("P: producer %d, produced total is: %d \n \n", thrd -> tid, gv -> producer_count);
				}
			} 
		}
		// release the lock
		V(mutex);
		V(fullCount);
	
		// if the number of items reaches the max, break and terminates
		if(gv -> producer_count >= gv -> total_items_2_produce) {
			// wake up all blocked consumer threads
			for (i = 0; i < gv -> num_c_thrds; i++) {
				printf("P: gv -> num_c_thrds is: %d \n", gv -> num_c_thrds);
				V(fullCount);
			}
			printf("P: Producer %d terminates \n \n", thrd -> tid);
			break;
		}
	}
}

