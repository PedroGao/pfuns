#include <stdio.h>
#include <malloc.h>

int main(int argc, char const *argv[])
{
  char *p = malloc(64);
  free(p);
  return 0;
}