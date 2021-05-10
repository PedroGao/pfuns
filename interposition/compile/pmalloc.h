#define malloc(size) pmalloc(size)

void *pmalloc(size_t size);