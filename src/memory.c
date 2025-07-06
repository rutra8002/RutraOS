#include "memory.h"
#include "terminal.h"

// Memory pool - our simple heap
static char memory_pool[MEMORY_POOL_SIZE];
static memory_block_t* memory_list = NULL;
static int memory_initialized = 0;

// Memory statistics
static size_t total_allocated = 0;
static size_t total_free = 0;
static size_t allocation_count = 0;

void memory_init(void) {
    if (memory_initialized) {
        return;
    }
    
    // Initialize the first memory block
    memory_list = (memory_block_t*)memory_pool;
    memory_list->size = MEMORY_POOL_SIZE - MEMORY_BLOCK_SIZE;
    memory_list->is_free = 1;
    memory_list->next = NULL;
    
    total_free = memory_list->size;
    memory_initialized = 1;
    
    terminal_writestring("Memory manager initialized with ");
    // Simple integer to string conversion for debugging
    char size_str[32];
    uint32_to_string(MEMORY_POOL_SIZE, size_str);
    terminal_writestring(size_str);
    terminal_writestring(" bytes\n");
}

void* kmalloc(size_t size) {
    if (!memory_initialized) {
        memory_init();
    }
    
    if (size == 0) {
        return NULL;
    }
    
    // Align size to 4 bytes for better performance
    size = (size + 3) & ~3;
    
    memory_block_t* current = memory_list;
    
    // First-fit allocation strategy
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Split the block if it's much larger than needed
            if (current->size > size + MEMORY_BLOCK_SIZE + 16) {
                memory_block_t* new_block = (memory_block_t*)((char*)current + MEMORY_BLOCK_SIZE + size);
                new_block->size = current->size - size - MEMORY_BLOCK_SIZE;
                new_block->is_free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->is_free = 0;
            total_allocated += current->size;
            total_free -= current->size;
            allocation_count++;
            
            return (char*)current + MEMORY_BLOCK_SIZE;
        }
        current = current->next;
    }
    
    // No suitable block found
    return NULL;
}

void kfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    memory_block_t* block = (memory_block_t*)((char*)ptr - MEMORY_BLOCK_SIZE);
    
    // Validate that this is actually a block header
    if (block < (memory_block_t*)memory_pool || 
        (char*)block >= memory_pool + MEMORY_POOL_SIZE) {
        return; // Invalid pointer
    }
    
    if (block->is_free) {
        return; // Already freed
    }
    
    block->is_free = 1;
    total_allocated -= block->size;
    total_free += block->size;
    allocation_count--;
    
    // Coalesce adjacent free blocks
    memory_block_t* current = memory_list;
    while (current != NULL && current->next != NULL) {
        if (current->is_free && current->next->is_free) {
            // Merge current with next
            current->size += current->next->size + MEMORY_BLOCK_SIZE;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void* memset(void* ptr, int value, size_t size) {
    unsigned char* p = (unsigned char*)ptr;
    while (size--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t size) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (size--) {
        *d++ = *s++;
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t size) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    
    while (size--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

void memory_print_stats(void) {
    terminal_writestring("=== Memory Statistics ===\n");
    
    // Heap statistics
    terminal_writestring("HEAP MEMORY:\n");
    terminal_writestring("  Allocated: ");
    char buffer[32];
    uint32_to_string(total_allocated, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    terminal_writestring("  Free: ");
    uint32_to_string(total_free, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    terminal_writestring("  Active allocations: ");
    uint32_to_string(allocation_count, buffer);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    // Kernel memory usage (actual values)
    terminal_writestring("\nKERNEL MEMORY:\n");
    
    // For GRUB-loaded kernels, we need to be more careful about BSS calculation
    // The BSS section includes our memory pool, so we need to subtract it
    
    // Calculate actual size of each section (excluding padding)
    uint32_t text_size = (uint32_t)((uintptr_t)text_end - (uintptr_t)text_start);
    uint32_t rodata_size = (uint32_t)((uintptr_t)rodata_end - (uintptr_t)rodata_start);
    uint32_t data_size = (uint32_t)((uintptr_t)data_end - (uintptr_t)data_start);
    uint32_t bss_total = (uint32_t)((uintptr_t)bss_end - (uintptr_t)bss_start);
    
    // The BSS section includes our memory pool - subtract it to get actual kernel BSS
    uint32_t bss_size = bss_total - MEMORY_POOL_SIZE;
    
    // Calculate total actual kernel size (without our memory pool)
    uint32_t kernel_size = text_size + rodata_size + data_size + bss_size;
    
    terminal_writestring("  Text section: ");
    uint32_to_string(text_size, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    terminal_writestring("  Read-only data: ");
    uint32_to_string(rodata_size, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    terminal_writestring("  Data section: ");
    uint32_to_string(data_size, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    terminal_writestring("  BSS section: ");
    uint32_to_string(bss_size, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes (excluding heap pool)\n");
    
    terminal_writestring("  Total Code + Data: ");
    uint32_to_string(kernel_size, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    // Show memory layout for debugging
    terminal_writestring("  Memory layout:\n");
    terminal_writestring("    Kernel start: 0x");
    uint32_to_hex((uint32_t)(uintptr_t)kernel_start, buffer);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    terminal_writestring("    Kernel end: 0x");
    uint32_to_hex((uint32_t)(uintptr_t)kernel_end, buffer);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    // Get current stack pointer
    uint64_t stack_pointer;
    __asm__ volatile("mov %%rsp, %0" : "=r"(stack_pointer));
    
    terminal_writestring("    Current stack: 0x");
    uint32_to_hex((uint32_t)stack_pointer, buffer);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    // For GRUB, the stack is managed differently
    // Let's use a more conservative approach based on typical GRUB behavior
    uint32_t stack_used = 8192; // 8KB conservative estimate for GRUB environment
    
    terminal_writestring("  Stack used: ");
    uint32_to_string(stack_used, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes (est)\n");
    
    terminal_writestring("  Heap Pool: ");
    uint32_to_string(MEMORY_POOL_SIZE, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    // Calculate total used with actual measured values
    uint32_t total_used = kernel_size + stack_used + total_allocated;
    terminal_writestring("\nTOTAL SYSTEM MEMORY:\n");
    terminal_writestring("  Used: ");
    uint32_to_string(total_used, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    terminal_writestring("  Available for allocation: ");
    uint32_to_string(total_free, buffer);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    // Show memory blocks
    terminal_writestring("\nHEAP BLOCKS:\n");
    memory_block_t* current = memory_list;
    int block_count = 0;
    
    while (current != NULL && block_count < 10) { // Limit output
        terminal_writestring("  Block ");
        uint32_to_string(block_count, buffer);
        terminal_writestring(buffer);
        terminal_writestring(": ");
        uint32_to_string(current->size, buffer);
        terminal_writestring(buffer);
        terminal_writestring(" bytes");
        terminal_writestring(current->is_free ? " (FREE)" : " (USED)");
        terminal_writestring("\n");
        
        current = current->next;
        block_count++;
    }
}

// Helper function to convert uint32 to string
void uint32_to_string(uint32_t value, char* buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    int i = 0;
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    buffer[i] = '\0';
    
    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - 1 - j];
        buffer[i - 1 - j] = temp;
    }
}

// Helper function to convert uint32 to hex string
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
