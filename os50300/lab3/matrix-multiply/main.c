/*****************************************************************************
 * CSCI 503 Operating System Lab 3 part 2: Multi-threads matrix multiplication
 * 
 *
 * @author: Xiaoyang Zhong
 *****************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include "do_thread.h"

/**
 * Initialization of thread information structure
 * 
 * @arguments 
 *		LV lv: thread information structure
 *		int tid: thread id
 *		GV gv: globle variables
 *		double **a:
 *		double **b:
 *		double **c:	matrices
 *		int **flag: flags for matrix c, indicating whether the element c[i][j] is computed
 *		Sem s: semaphore, used as a mutex
**/
void init_thrd(LV lv, int tid, GV gv, double **a, double **b, double **c, int **flag, Sem s) {
	lv -> tid = tid;
	lv -> gv = gv;
	lv -> A = a;
	lv -> B = b;
	lv -> C = c;
	lv -> C_flag = flag;
	lv -> mutex = s;
}

/**
 * Initialization of matrix
 * 
 * @arguments 
 *		int size: the size of N by N matirx
 *		int flag: if flag is 1, initialize A and B
 *				  else, initialize C and sequential C
 * @return
 *		double**: pointer to the matrix
**/
double** init_matrix(int size, int flag) {
	double** M;
	int k, j;
	M = (double**) malloc(sizeof(double*) * size);
	for(k = 0; k < size; k++) {
		*(M + k) = (double*) malloc(sizeof(double) * size);
	}
	// initialize using random numbers between 0.0 ~ 1.0
	srand((unsigned int)time(NULL));
	for (k = 0; k < size; k++)
		for (j = 0; j < size; j++ ) 
			if (flag == 1)		// inilization for A, B
				M[k][j] = (double) rand() / RAND_MAX;
			else				// inilization C and seqC
				M[k][j] = -1;
			
	return M;
}

/**
 * Initialization of flag matrix. Flag matrix is used to mark whether an element
 * in C is computed. If it is, we don't compute it again.
 * 
 * @arguments 
 *		int size: the size of N by N matrix
 * @return
 *		int** : pointer to the flag matrix
**/
int** init_flag(int size) {
	int** F;
	int k, j;
	F = (int**) malloc(sizeof(int*) * size);
	for(k = 0; k < size; k++) {
		*(F + k) = (int*) malloc(sizeof(int) * size);
	}
	for (k = 0; k < size; k++)
		for (j = 0; j < size; j++ ) 
			F[k][j] = 0;	// before computing, each flag is 0, after computing, turns to 1
			
	return F;
}

/**
 * Initialization of semaphore
 * 
 * @arguments 
 *		Sem s: the semaphore to be initialized
 *		int number: the initial value of the semaphore
**/
void init_sem(Sem s, int number) {
	s -> S = number;
	pthread_mutex_init(&s->lock, NULL);	// initialize lock to S
	pthread_cond_init(&s->cv, NULL);	// initialize conditional variable to S
}

/**
 * Main function, initialize all data structures for computing
 * 
 * First, create numbers of threads, and do multi-threads computing
 *
 * Second, sequentially compute A*B, store to seqC
 *
 * Last, compare the results of two methods, if they match, print a SUCCESS message
 * if they don't match, print unmatched items, and ERROR messages
 * 
**/
int main(int argc, char* argv[]) {
	int 			i, j, k;
	int				correct;	// Whether the two results matches or not
	int 			thrd_number;
	int				N;			// matrix size
	
	LV				lvs;		// threads information
	pthread_t		*thrds;		// producer threads
	pthread_attr_t  *attrs;
	
	// shared memories
	GV				gv;
	double 			**A;	// matrices
	double			**B;
	double			**C;
	int				**C_flag;	// if C[i][j] has computed, C_flag[i][j] = 1, otherwise 0;
	double			**seqC;
	
	void            *retval;	// used in "join"
	
	// mutex
	Sem mutex;
	
	if(argc != 3) {
		fprintf(stderr, "Usage: %s Matrix_Size Threads_Number \n", argv[0]);
		exit(1);
	}
	
	mutex = (Sem) malloc(sizeof(*mutex));	// mutex
	init_sem(mutex, 1);
	
	correct = 1;
	N = atoi(argv[1]);		// get matrix size and threads number
	thrd_number = atoi(argv[2]);
	
	// allocate memory space for variables
	lvs   = (LV) malloc(sizeof(*lvs)*thrd_number);
	thrds = (pthread_t*) malloc(sizeof(pthread_t)*thrd_number);
	attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*thrd_number);
	
	// gv and initialization
	gv    = (GV) malloc(sizeof(*gv));	
	gv -> i = 0;
	gv -> j = 0;
	gv -> N = N;
	pthread_mutex_init(&gv -> lock, NULL);
	
	// allocate memory space for matrices
	A = init_matrix(N, 1);
	B = init_matrix(N, 1);
	C = init_matrix(N, 0);
	seqC = init_matrix(N, 0);
	
	// flag matrix for C
	C_flag = init_flag(N);

	printf("Main: start multi-threads computing... \n");	
	// initialize and create threads
	for(i = 0; i < thrd_number; i++) {
		init_thrd(lvs+i, i, gv, A, B, C, C_flag, mutex);
		if(pthread_attr_init(attrs+i)) 
			perror("attr_init()");
		if(pthread_attr_setscope(attrs+i, PTHREAD_SCOPE_SYSTEM)) 
			perror("attr_setscope()");
		// create threads
		if(pthread_create(thrds+i, attrs+i, do_thread, lvs+i)) {
			perror("pthread_create()");
			exit(1);
		} else {
			printf("Main: create thread %d SUCCESS!\n", i);
		}
	}
	
	// wait threads to terminate
	for(i = 0; i < thrd_number; i++) {
		pthread_join(thrds[i], &retval);
		printf("Thread %d is finished\n \n", i);
	}
	
	printf("Main: multi-threads computed complete, start sequential computing... \n \n");
	// compute sequential C
	for (i = 0; i < N; i++) 
		for(j = 0; j < N; j++){
			seqC[i][j] = 0; 
			for(k = 0; k < N; k++)
				seqC[i][j] = seqC[i][j] + A[i][k] * B[k][j];
		}
	printf("Main: Both computing completed, start checking error... \n \n");
	// check errors
	for (i = 0; i < N; i++)
		for(j = 0; j < N; j++) {
			if(C[i][j] != seqC[i][j]) {
				printf("Error: C[%d][%d] is: %f, seqC[%d][%d] is: %f \n", i, j, C[i][j], i, j, seqC[i][j]);
				correct = 0;
			}
		}
	
	// print check result
	if(!correct) {
		printf("Error: Sequential and multi-threads matrix multiplication NOT match!\n ");
	} else {
		printf("SUCCESS: Multi_threads matrix multiplication CORRECT! \n");
	} 
	
	// free all memories
	free(lvs);
	free(attrs);
	free(thrds);
	free(gv);
	for(i = 0; i < N; i++) {
		free(*(A + i));
		free(*(B + i));
		free(*(C + i));
		free(*(seqC + i));
	}
		
	free(A);
	free(B);
	free(C);
	free(seqC);

	return 0;
}

