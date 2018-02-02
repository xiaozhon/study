#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/* Try to answer the two questions asked in the program. */

void* do_thread(void* lv) {
  int* tid;
  int  i;

  tid = (int*) lv;
 
  for(i = 0; i < 10; ++i) {
    printf("Thread %d[%d]: Hello, world!\n", *tid, i);
    sleep(1);
  }

  return NULL;
}

int main(int argc, char* argv[]) {
  int              i, n;
  int              *tids;  
  pthread_t        *thrds;
  pthread_attr_t   *attrs;
  void             *retval;

  if(argc != 2) {
    fprintf(stderr, "Usage: %s num_threads\n", argv[0]);
    exit(1);
  }

  n = atoi(argv[1]);

  thrds = (pthread_t*) malloc(sizeof(pthread_t)*n);
  attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*n);
  tids  = (int*) malloc(sizeof(int)*n);

  // create threads
  for(i = 0; i < n; i++) {
    if(pthread_attr_init(attrs+i)) {
        perror("attr_init()");
    }
    //if(pthread_attr_setscope(attrs+i, PTHREAD_SCOPE_SYSTEM)) perror("attr_setscope()");
    tids[i] = i;
    if(pthread_create(thrds+i, attrs+i, do_thread, tids+i) != 0) {
      perror("pthread_create()");
      exit(1);
    }
  }

  // join threads
  for(i = 0; i < n; i++) { // Q1: What would happen if remove all joins?
    pthread_join(thrds[i], &retval);	// all the threads will only print "hello world" once
										// main thread will terminates, thus other thread will terminates
  }


  free(attrs);
  free(thrds);
  free(tids);

  return 0;  // Q2: What if we replace it by pthread_exit();
//  pthread_exit(&retval);	// the program runs normally, even if we commemt out pthread_join statement
}
