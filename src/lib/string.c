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

// Find last occurrence of character in string
char* strrchr(const char* s, int c) {
    const char* last = NULL;
    while (*s) {
        if (*s == c) {
            last = s;
        }
        s++;
    }
    return (*s == c) ? (char*)s : (char*)last;
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

// Convert uint32_t to string
void uint32_to_string(uint32_t value, char* buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    char temp[16];
    int i = 0;
    
    // Extract digits in reverse order
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Reverse the string
    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

// String tokenizer function
char* strtok(char* str, const char* delim) {
    static char* last_token = NULL;
    char* start;
    char* end;
    
    // Use provided string or continue from last token
    if (str != NULL) {
        last_token = str;
    } else if (last_token == NULL) {
        return NULL;
    }
    
    start = last_token;
    
    // Skip leading delimiters
    while (*start && strchr(delim, *start)) {
        start++;
    }
    
    // If we reached end of string, no more tokens
    if (*start == '\0') {
        last_token = NULL;
        return NULL;
    }
    
    // Find end of current token
    end = start;
    while (*end && !strchr(delim, *end)) {
        end++;
    }
    
    // If we found a delimiter, replace it with null terminator
    if (*end) {
        *end = '\0';
        last_token = end + 1;
    } else {
        last_token = NULL;
    }
    
    return start;
}

// Convert uint32_t to hex string
void uint32_to_hex(uint32_t value, char* buffer) {
    const char hex_chars[] = "0123456789ABCDEF";
    
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    int i = 0;
    while (value > 0) {
        buffer[i++] = hex_chars[value & 0xF];
        value >>= 4;
    }
    buffer[i] = '\0';
    
    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - 1 - j];
        buffer[i - 1 - j] = temp;
    }
}

// Convert string to integer
int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

// Convert uint32_t to string with padding
void uint32_to_string_padded(uint32_t value, char* buffer, int width, char pad) {
    char temp[32];
    uint32_to_string(value, temp);
    
    int len = strlen(temp);
    int pad_len = width - len;
    
    if (pad_len < 0) pad_len = 0;
    
    int i = 0;
    for (; i < pad_len; i++) {
        buffer[i] = pad;
    }
    
    strcpy(buffer + i, temp);
}
