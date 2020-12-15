#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

// 在堆里面分配可执行内存，即动态添加代码区
void *alloc_executable_memory(size_t size)
{
    void *ptr = mmap(0, size,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1,
                     0);
    // 如果返回 -1，证明内存分配失败
    if (ptr == (void *)-1)
    {
        perror("mmap");
        return NULL;
    }
    return ptr;
}

void emit_code_into_memory(unsigned char *m)
{
    unsigned char code[] =
        {
            0x48, 0x89, 0xf8,       // mov %rdi, %rax
            0x48, 0x83, 0xc0, 0x04, // add $4, %rax   ; rax 寄存器是函数返回值
            0xc3                    // ret
        };
    // 将代码段拷贝到内存中
    memcpy(m, code, sizeof(code));
}

const size_t SIZE = 1024;
typedef long (*JitFunc)(long); // 函数指针

void run_from_rwx()
{
    void *m = alloc_executable_memory(SIZE);
    emit_code_into_memory(m);

    JitFunc func = m;
    long result = func(2);
    printf("result = %ld\n", result);
}

int main()
{
    run_from_rwx();
    return 0;
}
