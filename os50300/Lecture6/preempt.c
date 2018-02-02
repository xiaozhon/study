#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


/*
 * Create threads as many as the number of cores on your system.
 * Even if each thread is of an infinite loop, the main thread still has chances to say "Hello".
 * Conclusion: Linux is a preemptive OS.
 */

void* do_thread(void* lv) {
  int* tid;

  tid = (int*) lv;
  
  while(1); 

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

  while(1) {
    printf("Main Thread: Hello!\n");
    sleep(1);
  }

  // join threads
  for(i = 0; i < n; i++) {
    pthread_join(thrds[i], &retval);
  //  printf("Thread %d is finished\n", i);
  }


  free(attrs);
  free(thrds);
  free(tids);

  exit(0);
}
