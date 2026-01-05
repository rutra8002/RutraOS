#ifndef STRING_H
#define STRING_H

#include "types.h"

// String functions
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strchr(const char* s, int c);
char* strrchr(const char* s, int c);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
char* strtok(char* str, const char* delim);

// Number to string conversion
int atoi(const char* str);
void uint32_to_string(uint32_t value, char* buffer);
void uint32_to_string_padded(uint32_t value, char* buffer, int width, char pad);
void uint32_to_hex(uint32_t value, char* buffer);

#endif // STRING_H
