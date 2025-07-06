#include "string.h"

// String length function
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

// String comparison function
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// String comparison function (n characters)
int strncmp(const char* s1, const char* s2, size_t n) {
    while (n > 0 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    return (n == 0) ? 0 : (*(const unsigned char*)s1 - *(const unsigned char*)s2);
}

// Find character in string
char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == c) {
            return (char*)s;
        }
        s++;
    }
    return (*s == c) ? (char*)s : NULL;
}

// Copy string
char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++)) {
        // Copy until null terminator
    }
    return original_dest;
}

// Copy string with length limit
char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

// Concatenate strings
char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    // Move to end of dest
    while (*dest) {
        dest++;
    }
    // Copy src to end of dest
    while ((*dest++ = *src++)) {
        // Copy until null terminator
    }
    return original_dest;
}
