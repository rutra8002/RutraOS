#include "memory_utils.h"

// Set memory to a value
void* memset(void* ptr, int value, size_t size) {
    unsigned char* p = (unsigned char*)ptr;
    for (size_t i = 0; i < size; i++) {
        p[i] = (unsigned char)value;
    }
    return ptr;
}

// Copy memory
void* memcpy(void* dest, const void* src, size_t size) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
    return dest;
}

// Move memory (handles overlapping regions)
void* memmove(void* dest, const void* src, size_t size) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        // Copy forward
        for (size_t i = 0; i < size; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        // Copy backward
        for (size_t i = size; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    return dest;
}

// Compare memory
int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}
