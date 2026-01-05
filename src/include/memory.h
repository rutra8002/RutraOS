#ifndef MEMORY_H
#define MEMORY_H

#include "kernel.h"

// Memory block structure for our simple allocator
typedef struct memory_block {
    size_t size;
    int is_free;
    struct memory_block* next;
} memory_block_t;

// Memory management constants
#define MEMORY_POOL_SIZE 1024 * 1024  // 1MB memory pool
#define MEMORY_BLOCK_SIZE sizeof(memory_block_t)

// Memory management functions
void memory_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);

// Memory debugging functions
void memory_print_stats(void);

#endif // MEMORY_H
