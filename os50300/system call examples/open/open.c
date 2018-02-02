#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

int main(int argc, char* argv[]) {
  int fd;
  
  // read file 1
  fd = open("open.c", O_RDONLY);
  if(fd < 0) {
    perror("open 1");
  } else {
    printf("open 1: success\n");
    close(fd);
  }

  // read file 2
  fd = open("notexit.c", O_RDONLY);
  if(fd < 0) {
    perror("open 2");
  }

  // create a new file
  fd = open("tmp.out", O_CREAT|O_TRUNC|O_WRONLY, 0644);
  if(fd < 0) {
    perror("open 3");
  } else {
    printf("open 3: success\n");
    close(fd);
  }
  
  return 0;

}
