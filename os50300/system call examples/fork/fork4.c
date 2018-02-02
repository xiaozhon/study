#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char* argv[]) {
  int ret;
  int x = 100, y = 200;
  int fd, seekp;
  char s[128];

  fd = open("tmp.out", O_WRONLY | O_TRUNC | O_CREAT, 0666);

  printf("Before fork: x = %d, y = %d\n", x , y);//try remove \n!

  sprintf(s, "%s", "Before fork\n");

  write(fd, s, strlen(s));

  ret = fork();

  if (ret > 0) {
    sleep(1);
    printf("Parent after fork: x = %d, y = %d\n", x, y);
    seekp = lseek(fd, 0, SEEK_CUR);
    sprintf(s, "Parent was here (file pointer postion=%d)\n", seekp);
  } else {
    x++; y++;
    printf("Child after fork: x = %d, y = %d\n", x, y);
    seekp = lseek(fd, 0, SEEK_CUR);
    sprintf(s, "Child was here (file pointer position=%d).\n", seekp);
  }
  
  
  write(fd, s, strlen(s));
  
  return 0; 
}
