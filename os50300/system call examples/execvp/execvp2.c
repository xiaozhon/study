#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

int main(int argc, char* argv[]) {
  int ret, status;
  int i = 0;
  char *newargv[3];

  newargv[0] = "cat";
  newargv[1] = "execvp1.c";
  newargv[2] = 0;


  for(i = 0; i < 3; i++) {
    printf("I am process %d, i=%d\n", getpid(), i);
    if(fork() == 0) {
      ret = execv("cat", newargv);
      if(ret < 0) exit(1);
    } else {
      wait(&status);
    }
  } 

  return 0;

}
