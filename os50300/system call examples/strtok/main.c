#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char* argv[]) {
  char input[] = "a 2 < | 4 b dfd | f | a > &";
  char* token;

  token = strtok(input, " ");
  printf("%s\n", token);

  while( (token = strtok(NULL, " ")) ) {
    printf("%s\n", token);
  }

  return 0;
}
