#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
  int ret1, ret2, status;

  ret1 = fork();

  if(ret1 > 0) {
    sleep(5);
    system("ps aux | grep fgsong");
    sleep(1);
    printf("\n\n");
    ret2 = wait(&status);
    printf("Parent: return value = %d\n", ret2);
    printf("Parent: status = %d\n", status);
    printf("Parent: WIFEXITED = %d\n", WIFEXITED(status));
    printf("Parent: WEXITSTATUS = %d\n", WEXITSTATUS(status));
  } else {
    printf("Child (PID=%d) will call exit(111)\n", getpid());
    exit(111);
  }

  return 0;

}

