#include <pthread.h>

/**
 * Data structures:
 *		thread private information
 *		shared matrices:
 *			matrix A
 *			matrix B
 *			matrix C
 *		globle data:
 *			index i, j
 *			lock for i, j
 *			matrix size N
**/
typedef struct lv_t {
	int tid;
	void *A;
	void *B;
	void *C;
	void *C_flag;
	void *gv;
	void *mutex;
}* LV;

typedef struct gv_t {
	int i;
	int j;
	pthread_mutex_t lock;
	int N;	// matrix size
}* GV;

typedef struct sem_t {
	int S;
	pthread_mutex_t lock;
	pthread_cond_t  cv;
}* Sem;

void* do_thread(void* v);


