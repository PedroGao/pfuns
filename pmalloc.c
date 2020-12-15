#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// 内存区域大致共有 5 个部分，分别如下：
// Text section：代码区，可执行的代码块
// Data section: 数据区，已初始化全局变量数据
// BSS：未初始化数据区
// Heap：堆区，内存动态分配
// Stack：栈区，函数调用自动管理内存区
// brk 是堆底的指针，所以分配内存需要增加 brk，而释放内存需要减少 brk
// 根据 brk 的移动来改变堆的大小，从而实现内存的分配和释放

// brk 可通过系统调用 sbrk 来实现，具体如下：
// sbrk(0) 返回当前 brk 指针的地址
// sbrk(x) x 为正数，会增加 brk 的数值，即分配内存，并且返回分配内存的首地址
// sbrk(-x) -x 为负数，会减少 brk 的数值，即释放内存，并且返回释放内存的首地址

typedef char ALIGN[16];

// 内存块头部
union header
{
    struct
    {
        size_t size;
        unsigned is_free;
        union header *next;
    } s;
    ALIGN stub; // 16 字节对齐
};

typedef union header header_t;

header_t *head, *tail;

pthread_mutex_t global_malloc_lock; // 全局锁

/* 获得空闲内存块 */
header_t *get_free_block(size_t size)
{
    header_t *curr = head; // 头节点
    while (curr)
    {
        // 如果 curr 是 free 且块的大小 >= size，则将当前的块返回
        if (curr->s.is_free && curr->s.size >= size)
        {
            return curr;
        }
        curr = curr->s.next;
    }
    // 如果没有内存块满足，则返回 NULL
    return NULL;
}

/* 分配内存 */
void *malloc(size_t size)
{
    size_t total_size;
    void *block;
    header_t *header;
    if (!size)
        return NULL;
    pthread_mutex_lock(&global_malloc_lock); // 加锁
    header = get_free_block(size);           // 获得空闲块
    if (header)                              // 如果得到了空闲块
    {
        header->s.is_free = 0;                     // 块已经使用
        pthread_mutex_unlock(&global_malloc_lock); // 解锁
                                                   // 注意：这里是 header + 1，因此 header 是 header的头指针
                                                   // 但是我们需要拿到真正的内存块，所以 header + 1，然后转化为 void*
        return (void *)(header + 1);
    }
    // 如果没有空闲的
    // 总大小
    total_size = sizeof(header_t) + size;
    // 通过 sbrk 系统调用来分配
    block = sbrk(total_size);
    // 如果 sbrk 返回 -1，则代表内存失败，所以返回 NULL
    if (block == (void *)-1)
    {
        // 注意：在返回之前一定记得释放全局锁
        // 从这里就可以看出 go 语言中的 defer 有多么好用了，不用到处写 unlock
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    // block 是总大小，包括的 header_size + body_size
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL; // 因为是新分配的内存块，所以 next 是 NULL
    if (!head)             // 如果当前还没有头节点，则设置头节点
    {
        head = header;
    }
    if (tail) // 如果有尾节点，尾节点 next 为当前节点
    {
        tail->s.next = header;
    }
    tail = header; // 并且更新当前节点为尾节点
    // 解锁，返回内存块指针
    pthread_mutex_unlock(&global_malloc_lock);
    return (void *)(header + 1);
}

/* 释放内存块 */
void free(void *block)
{
    header_t *header, *tmp;
    void *programbreak;
    if (!block) // NULL 直接 return
    {
        return;
    }
    // 全局加锁，主要是为了锁住链表
    pthread_mutex_lock(&global_malloc_lock);
    // block 是 body 的头指针，所以将其转化为 header_t 头节点
    header = (header_t *)block - 1;
    // 当前 brk 指向的地址
    programbreak = sbrk(0);
    // 如果当前块的末尾指针就是 brk 的地址，即当前块挨着 brk
    // 则释放当前块，因为这个块就在 brk 旁边，方便释放
    // size_t total_size = header->s.size + sizeof(header_t);
    if ((char *)block + header->s.size == programbreak)
    {
        // 如果头、尾节点一样，即只有一块内存
        if (head == tail)
        {
            head = tail = NULL; // 将头、尾节点设置为 NULL
        }
        else
        {
            tmp = head;
            while (tmp)
            {
                // 如果当前节点的下一个节点是尾节点
                if (tmp->s.next == tail)
                {
                    // 懂了：因为 block 在 brk 旁边，所以 tail 节点就是 block 节点
                    // 要释放 tail 节点，需要拿到 tail 前面的一个节点
                    // 但这是单向链表，所以从头开始找这个节点
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                // tmp 指向下一个
                tmp = tmp->s.next;
            }
        }
        // 负数，把 brk 指针向下移动，即释放内存
        // 注意：此处释放的内存大小是，是 header_size + body_size
        sbrk(0 - sizeof(header_t) - header->s.size);
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }
    // 如果不是在 brk 旁边，则比较麻烦，所以单纯的将其标记为 free 即可
    header->s.is_free = 1;                     // 设置标志位
    pthread_mutex_unlock(&global_malloc_lock); // 解锁
}

/* 分配 num * nsize 内存 */
void *calloc(size_t num, size_t nsize)
{
    size_t size;
    void *block;
    if (!num || !nsize)
        return NULL;
    size = num * nsize;
    /* check mul overflow */
    if (nsize != size / num)
        return NULL;
    block = malloc(size);
    if (!block)
        return NULL;
    // 这个地方多了一个清 0 操作
    memset(block, 0, size);
    return block;
}

/* 重新分配内存 */
void *realloc(void *block, size_t size)
{
    header_t *header;
    void *ret;
    if (!block || !size)
        return malloc(size);
    header = (header_t *)block - 1;
    // 如果当前块的内存本来就 >= size，直接返回这个块即可
    if (header->s.size >= size)
        return block;
    // 重新分配内存
    ret = malloc(size);
    if (ret)
    {
        // 拷贝内存
        memcpy(ret, block, header->s.size);
        // 释放掉原来的内存
        free(block);
    }
    return ret;
}

int main()
{
    // 这是一个简单的单链表内存管理器 allocator 实现
    // 详细内容可参考 CSAPP 内存管理一章
    // 分配 120 字节内存
    void *ptr = malloc(120);
    // 得到头部数据
    // header_t *header = (header_t *)ptr - 1;
    // printf("block size shoud be 120, and size is true: %ld.\n", header->s.size);
    free(ptr);
    // printf("after free memory, head and tail pointer should be null.\n");
    // if (head != NULL || tail != NULL)
    // {
    //     printf("No, they are also here.\n");
    // }
    // else
    // {
    //     printf("Yes, they are null.\n");
    // }

    return 0;
}
