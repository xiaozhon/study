#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  int  pipefd[2];
  char str1[] = "Hello, IUPUI!\n";
  char str2[256];
  int  n;

  if(pipe(pipefd) < 0) {
    perror("pipe"); exit(1);
  }
 
  write(pipefd[1], str1, strlen(str1));

  n = read(pipefd[0], str2, sizeof(str2)-1);
  str2[n] = 0;

  printf("Content read from the pipe: %s", str2);

  return 0;
}
