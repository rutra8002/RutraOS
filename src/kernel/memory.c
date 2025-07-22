#include "../include/memory.h"
#include "terminal.h"
#include "string.h"
#include "memory_utils.h"

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
    
    // Calculate actual stack usage by finding stack boundaries
    // The stack grows downward, so we need to find where it started
    uint64_t kernel_end_addr = (uint64_t)(uintptr_t)kernel_end;
    uint32_t stack_used;
    
    // Method 1: If stack is reasonably close to kernel end, measure from there
    if (stack_pointer > kernel_end_addr && (stack_pointer - kernel_end_addr) < 0x100000) {
        // Stack appears to be right after kernel, this is actual usage
        stack_used = (uint32_t)(stack_pointer - kernel_end_addr);
        terminal_writestring("  Stack used: ");
        uint32_to_string(stack_used, buffer);
        terminal_writestring(buffer);
        terminal_writestring(" bytes (measured from kernel end)\n");
    } else {
        // Method 2: Calculate based on actual stack frame measurements
        // Count stack frames by walking the stack and measure actual usage
        uint64_t* frame_ptr;
        __asm__ volatile("mov %%rbp, %0" : "=r"(frame_ptr));
        
        int frame_count = 0;
        uint64_t* current_frame = frame_ptr;
        uint64_t* first_frame = frame_ptr;
        uint64_t* last_frame = frame_ptr;
        
        // Walk the stack frames (carefully) to find actual boundaries
        while (current_frame != NULL && frame_count < 50) {
            // Check if the frame pointer looks valid
            if ((uint64_t)current_frame < stack_pointer || 
                (uint64_t)current_frame > stack_pointer + 0x10000) {
                break;
            }
            
            uint64_t* next_frame = (uint64_t*)*current_frame;
            if (next_frame <= current_frame) {
                break; // Prevent infinite loops
            }
            
            last_frame = current_frame;
            current_frame = next_frame;
            frame_count++;
        }
        
        // Calculate actual stack usage from measured frame boundaries
        if (frame_count > 0 && last_frame > first_frame) {
            // Measure actual distance between first and last frame
            stack_used = (uint32_t)((uint64_t)last_frame - (uint64_t)first_frame);
            // Add space for current frame (from RSP to first frame)
            stack_used += (uint32_t)((uint64_t)first_frame - stack_pointer);
        } else {
            // Fallback: measure from stack pointer to frame pointer
            stack_used = (uint32_t)((uint64_t)frame_ptr - stack_pointer);
        }
        
        terminal_writestring("  Stack used: ");
        uint32_to_string(stack_used, buffer);
        terminal_writestring(buffer);
        terminal_writestring(" bytes (measured from ");
        uint32_to_string(frame_count, buffer);
        terminal_writestring(buffer);
        terminal_writestring(" frames)\n");
    }
    
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
