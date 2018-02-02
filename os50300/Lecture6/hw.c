#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


/* Your first multi-threaded "Hello World" program */

void* do_thread(void* lv) {
  int* tid;

  tid = (int*) lv;
  printf("Thread %d: Hello, world!\n", *tid);

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

  thrds = (pthread_t*) malloc(sizeof(pthread_t)*n);			// malloc space
  attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*n);
  tids  = (int*) malloc(sizeof(int)*n);

  // create threads
  for(i = 0; i < n; i++) {
    if(pthread_attr_init(attrs+i)) {
        perror("attr_init()");
    }
    //if(pthread_attr_setscope(attrs+i, PTHREAD_SCOPE_SYSTEM)) perror("attr_setscope()");
    tids[i] = i;
    if(pthread_create(thrds+i, attrs+i, do_thread, tids+i) != 0) { // Q: if replace tids+i by &i.
										// the thread will execute "do_thread"
      perror("pthread_create()");
      exit(1);
    }
  }

  // join threads
  for(i = 0; i < n; i++) {
    pthread_join(thrds[i], &retval);
    //printf("Thread %d is finished\n", i);
  }


  free(attrs);
  free(thrds);
  free(tids);

  exit(0);
}
