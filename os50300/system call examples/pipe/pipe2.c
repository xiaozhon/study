#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/* Run "ps -u", the child process is still running. Why? */

int main(int argc, char* argv[]) {
  int  pipefd[2];
  char str[1024];
  int  i, line;

  if(pipe(pipefd) < 0) {
    perror("pipe"); exit(1);
  }
 

  if(fork() > 0) {
    printf("parent pid: %d\n", getpid());
    //Parent: read a line from stdin, write it to pipe.
    while(fgets(str, sizeof(str), stdin) != NULL) {
      write(pipefd[1], str, strlen(str));
    }
    close(pipefd[1]);
  } else {
    printf("child pid: %d\n", getpid());
    //Child: read a line from pipe, and print it out.
    i    = 0;
    line = 0;
    while(read(pipefd[0], str+i, 1) == 1) {
      if (str[i] == '\n') {
        line++; //reach the end of a new line.
        str[i] = '\0';
        printf("%2d  %s\n", line, str);
        i = 0; // a new line will start next.
      } else {
        i++;
      }
    }
  }
 
  return 0;
}
