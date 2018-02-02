#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  int fd1, fd2;
  char str1[] = "AAAAA";
  char str2[] = "BBBBBBBBBB";

  fd1 = open("tmp.out", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  fd2 = dup(fd1);

  write(fd1, str1, strlen(str1));
  write(fd2, str2, strlen(str2)); 
  
  close(fd1);
  close(fd2);

  //Look at fork3.c to see how fork affects file pointer.

  return 0;
}
