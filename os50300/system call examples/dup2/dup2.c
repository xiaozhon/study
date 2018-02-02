#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
  int fd;

  fd = open("tmp.out", O_WRONLY | O_CREAT | O_TRUNC, 0644);

  if(dup2(fd, 1) < 0) {
    perror("dup2"); exit(1);
  }

  printf("Hello world!\n");
  

  return 0;
}

