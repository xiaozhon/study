#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  int ret;

  printf("Hello, I am process %d.\n", getpid());
  
  ret = fork();
  
  printf("Hello, I am process %d: parent=%d, ret value=%d\n", 
          getpid(), getppid(), ret);  

  
  return 0;
}
