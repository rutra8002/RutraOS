#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

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
