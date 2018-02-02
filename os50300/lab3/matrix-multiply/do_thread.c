#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "do_thread.h"

/**
 * Semaphore operation: P
 * 
 * @arguments 
 *		Sem s: the semaphore
**/

void P(Sem s) {
//	printf("I am in P \n");
	pthread_mutex_lock(&s -> lock);	// we need this lock?
	
	while (s -> S <= 0) {
		pthread_cond_wait(&s -> cv, &s -> lock);
	}
	s -> S--;

	pthread_mutex_unlock(&s -> lock);
}

/**
 * Semaphore operation: V
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
 * Thread routine
**/
void* do_thread(void *v) {
	LV lv;
	GV gv;
	double **A;
	double **B;
	double **C;
	int **C_flag;
	int m, n;	// use to store index of C's element to be computed
	int k;
	Sem mutex;
	
	// get pointers to shared memory
	lv = (LV) v;
	gv = (GV) lv -> gv;
	A = (double**) lv -> A;
	B = (double**) lv -> B;
	C = (double**) lv -> C;
	C_flag = (int**) lv -> C_flag;
	
	mutex = (Sem) lv -> mutex;
	
	// while loop, where real work is done
	while(1) {
	    // lock the access to C's index i, j
		P(mutex);
		// we only compute the result when we have not reached the end of the matrix
		if(gv -> i < gv -> N) {			
			if(gv -> j == (gv -> N)) {	// reaches the end of the row, start next row
				gv -> i++;
				m = gv -> i;
				n = gv -> j = 0;
				gv -> j++;
			} else {	// in the same row
				m = gv -> i;
				n = gv -> j;
				gv -> j++;
			}
		}
		// release lock
		V(mutex);
		
		// if we have not reached the end, and C[m][n] has not been computed yet
		if(m < gv -> N) {	
			if(C_flag[m][n] == 0){	// check C_flag
				C[m][n] = 0;
				for(k = 0; k < gv -> N; k++)
					C[m][n] = C[m][n] + A[m][k] * B[k][n];
				C_flag[m][n] = 1;
			} 
		}
		
		// if reaches the end, terminates
		if(gv -> i >= gv -> N){
			printf("Thread %d reaches the end, terminate! \n", lv -> tid);
			break;
		}
		
	}
}


