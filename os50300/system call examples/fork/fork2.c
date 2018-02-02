#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  int ret;

  printf("Hello, I am process %d.\n", getpid());
  
  ret = fork();

  if(ret == 0) {  
    printf("Hello, I am process %d: parent=%d, ret value=%d\n", 
           getpid(), getppid(), ret);  

  } else {
    printf("Hello, I am process %d: parent=%d, ret value=%d\n",
           getpid(), getppid(), ret);
    sleep(5);
  }
  
  return 0;
}
