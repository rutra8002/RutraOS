#ifndef TYPES_H
#define TYPES_H

// Standard types for our kernel
typedef unsigned long size_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned long uintptr_t;  // For pointer arithmetic

// Define NULL since we don't use standard libraries
#ifndef NULL
#define NULL ((void*)0)
#endif

#endif // TYPES_H
