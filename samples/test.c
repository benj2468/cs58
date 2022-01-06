#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define LEN0 (2 << 25)
#define LEN1 30

char dest[LEN0];

int main()
{

  pid_t i;
  int n;
  char *src = "there is no dark side of the moon, really";
  void *ptr;

  printf("testing getpid, strncpy, malloc(%d), malloc(%d)\n", LEN0, LEN1);

  i = getpid();

  strncpy(dest, src, LEN0);

  ptr = malloc(LEN0);

  ptr = malloc(LEN1);
}
