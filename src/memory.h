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

// Memory utility functions
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);
int memcmp(const void* s1, const void* s2, size_t size);

// Memory debugging functions
void memory_print_stats(void);

// Helper functions
void uint32_to_string(uint32_t value, char* buffer);
void uint32_to_hex(uint32_t value, char* buffer);

#endif // MEMORY_H
