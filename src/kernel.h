#ifndef KERNEL_H
#define KERNEL_H

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

// Linker symbols for kernel size calculation
extern char kernel_start[];
extern char kernel_end[];
extern char text_start[];
extern char text_end[];
extern char rodata_start[];
extern char rodata_end[];
extern char data_start[];
extern char data_end[];
extern char bss_start[];
extern char bss_end[];

// Function declarations
void kernel_main(void);

#endif // KERNEL_H
