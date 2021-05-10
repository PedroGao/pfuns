#ifdef PMALLOC
#include <stdlib.h>
#include <stdio.h>

void *__real_malloc(size_t size);

// 打桩函数
void *__wrap_malloc(size_t size)
{
  void *ptr = __real_malloc(size);
  printf("ptr is %p\n", ptr);
  return ptr;
}

#endif