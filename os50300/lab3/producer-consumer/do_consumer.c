#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "producer_consumer.h"

/**
 * Consumer thread routine
**/
void* do_consumer(void *v) {
	TD thrd;
	GV gv;
	Sem emptyCount;
	Sem fullCount;
	Sem mutex;
	ring_buffer buf;
	
	int sleep_t = 1;
	int i;
	// pointers to share memories
	thrd = (TD) v;
	gv = (GV) thrd -> gv;
	buf = (ring_buffer) thrd -> buf;
	emptyCount = (Sem) thrd -> emptyCount;
	fullCount = (Sem) thrd -> fullCount;
	mutex = (Sem) thrd -> mutex;

	// while loop, where real work is done
	while(1) {
		// consumer only gets in if consumed items count does not reach the max
	//	if(gv -> consumer_count < gv -> total_items_2_produce) {	// if

		P(fullCount);
		// lock buffer
		P(mutex);
		// get item from the buffer
		if(gv -> consumer_count < gv -> total_items_2_produce) {
			// if buffer full, do nothing
            // if we use "while" here, one producer may get the mutex and do busy waiting forever,
            // because no consumer can get into this section
            // if buffer size is 1, this is not a ring buffer
            if (buf -> size == 1){
                // use buf->head to indicate whether the item has been consumed
                // if buf->head = 0, the item has been consumed, otherwise it is still there
                // producer should do nothing
                if(buf -> head == 1){
                        buf -> head = 0;
                        gv -> consumer_count++;
                }
			} else {
			// buffer empty, do nothing
			if(buf -> head == buf -> tail) {
				printf("C: buffer empty, consumer %d do nothing \n \n", thrd -> tid);
			}else {
				buf -> tail = (buf -> tail + 1) % buf -> size;
				gv -> consumer_count++;
				printf("C: consumer: %d , consumed total is: %d \n \n", thrd -> tid, gv -> consumer_count);
			}
			}
		
		} 
		// release the lock
		V(mutex);
		
		V(emptyCount);
	
		// randomly sleep, consume
		sleep_t = (random() % gv -> max_sleep) + 1;	
		sleep(sleep_t);

		// if the number of items reaches the max, break and terminates
		if(gv -> consumer_count >= gv -> total_items_2_produce) {
			// wake up all blocked consumer thread
			for (i = 0; i < gv -> num_p_thrds; i++) {
				V(emptyCount);
			}
			printf("C: consumer %d terminates \n \n", thrd -> tid);
			break;
		}
	}
}

