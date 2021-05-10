#ifdef PMALLOC
#include <stdio.h>
#include <stdlib.h>

// 打桩函数
void *pmalloc(size_t size)
{
  void *ptr = malloc(size);
  printf("ptr is %p\n", ptr);
  return ptr;
}

#endif