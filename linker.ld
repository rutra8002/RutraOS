ENTRY(_start)

SECTIONS
{
    . = 1M;
    
    /* Mark the start of the kernel */
    kernel_start = .;

    .text BLOCK(4K) : ALIGN(4K)
    {
        text_start = .;
        *(.multiboot)
        *(.text)
        text_end = .;
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        rodata_start = .;
        *(.rodata)
        rodata_end = .;
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        data_start = .;
        *(.data)
        data_end = .;
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        bss_start = .;
        *(COMMON)
        *(.bss)
        bss_end = .;
    }
    
    /* Mark the end of the kernel */
    kernel_end = .;
}
