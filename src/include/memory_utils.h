#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include "types.h"

// Memory functions
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);
void* memmove(void* dest, const void* src, size_t size);
int memcmp(const void* s1, const void* s2, size_t n);

#endif // MEMORY_UTILS_H
