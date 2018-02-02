#include <stdio.h>
#include <omp.h>

int main(int argc, char* argv[]) {

  printf("There are %d threads.\n", omp_get_max_threads());
  
#pragma omp parallel 
{
  printf("Thread%d: Hello, world!\n", omp_get_thread_num());
}

  return 0;
}
