#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  int fd, ret;
  char str[256];

  fd = open("tmp.out", O_WRONLY | O_CREAT | O_TRUNC, 0644);

   
  ret = fork();

  if(ret == 0) {
    sprintf(str, "Hi, I am the child\n");
  } else {
    sprintf(str, "Hi, I am the parent\n");
    sleep(1);
  }

  write(fd, str, strlen(str));
  
  close(fd);

  return 0;
}
