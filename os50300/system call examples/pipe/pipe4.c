#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>


int main(int argc, char* argv[]) {
  int dummy2, dummy1, i;
  int pipefd[2];
  char* cmd1_argv[3] = {"cat", "pipe4.c", NULL};
  char* cmd2_argv[2] = {"sort", NULL};

  i = pipe(pipefd);
  if (i < 0) {
    perror("pipe"); exit(1);
  }

  /*if(fork() > 0) {
    close(pipefd[1]);
    close(pipefd[0]);
    exit(1);
  }*/

  if (fork() == 0) {
    //First child:
    if (dup2(pipefd[1], 1) != 1) {
      perror("dup2");
      exit(1);
    }
    close(pipefd[1]);
    close(pipefd[0]);
    if(execvp("cat", cmd1_argv) < 0) {
      perror("execvp"); exit(1);
    }
  } else if (fork() == 0) {
    //Second child:
    if (dup2(pipefd[0], 0) != 0) {   
      perror("dup2");
      exit(1);
    }
    close(pipefd[1]);
    close(pipefd[0]);
    if(execvp("sort", cmd2_argv) < 0) {
      perror("execvp"); exit(1);
    }
  } else {
    //The parent:
    close(pipefd[1]);
    close(pipefd[0]);
    wait(&dummy1);
    wait(&dummy2);
    if (WEXITSTATUS(dummy1) || WEXITSTATUS(dummy2)) {
      fprintf(stderr, "Abnormal exit.\n");
    }
  }

  return 0;
}
