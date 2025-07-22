#include "process.h"
#include "memory.h"
#include "terminal.h"
#include "string.h"
#include "memory_utils.h"

// Global process management data
process_t* process_list_head = NULL;  // Made non-static for external access
process_t* current_process = NULL;  // Made non-static for external access
static uint32_t next_pid = 1;
static int scheduler_initialized = 0;

// Time slice for round-robin scheduling (in milliseconds)
#define TIME_SLICE_MS 100
#define MAX_PROCESSES 64

static process_t process_pool[MAX_PROCESSES];
static int process_pool_index = 0;

// Initialize the process management system
void process_init(void) {
    if (scheduler_initialized) {
        return;
    }
    
    terminal_writestring("Initializing process management...\n");
    
    // Clear process pool
    memset(process_pool, 0, sizeof(process_pool));
    process_pool_index = 0;
    
    scheduler_initialized = 1;
    terminal_writestring("Process management initialized\n");
}

// Generate a unique process ID
uint32_t process_generate_pid(void) {
    return next_pid++;
}

// Allocate a process from the pool
static process_t* process_allocate(void) {
    if (process_pool_index >= MAX_PROCESSES) {
        return NULL;
    }
    
    process_t* proc = &process_pool[process_pool_index++];
    memset(proc, 0, sizeof(process_t));
    return proc;
}

// Add process to the linked list
static void process_add_to_list(process_t* proc) {
    if (!process_list_head) {
        process_list_head = proc;
        proc->next = proc;
        proc->prev = proc;
    } else {
        process_t* tail = process_list_head->prev;
        tail->next = proc;
        proc->prev = tail;
        proc->next = process_list_head;
        process_list_head->prev = proc;
    }
}

// Remove process from the linked list
static void process_remove_from_list(process_t* proc) {
    if (proc->next == proc) {
        // Only process in list
        process_list_head = NULL;
    } else {
        if (process_list_head == proc) {
            process_list_head = proc->next;
        }
        proc->prev->next = proc->next;
        proc->next->prev = proc->prev;
    }
    proc->next = NULL;
    proc->prev = NULL;
}

// Create a new process
process_t* process_create(const char* name, process_entry_t entry, void* args,
                         process_priority_t priority, size_t stack_size) {
    if (!scheduler_initialized && strcmp(name, "kernel") != 0) {
        return NULL;
    }
    
    // Allocate process structure
    process_t* proc = process_allocate();
    if (!proc) {
        terminal_writestring("ERROR: No free process slots\n");
        return NULL;
    }
    
    // Initialize process data
    proc->pid = process_generate_pid();
    strncpy(proc->name, name, sizeof(proc->name) - 1);
    proc->name[sizeof(proc->name) - 1] = '\0';
    proc->state = PROCESS_STATE_READY;
    proc->priority = priority;
    proc->time_slice = TIME_SLICE_MS;
    proc->time_used = 0;
    proc->total_time = 0;
    proc->exit_code = 0;
    proc->parent = current_process;
    
    // Allocate stack
    proc->stack_size = stack_size;
    proc->stack_base = kmalloc(stack_size);
    if (!proc->stack_base) {
        terminal_writestring("ERROR: Failed to allocate process stack\n");
        return NULL;
    }
    
    // Allocate process memory (simple linear allocation for now)
    proc->memory_size = 64 * 1024; // 64KB per process
    proc->memory_base = kmalloc(proc->memory_size);
    if (!proc->memory_base) {
        terminal_writestring("ERROR: Failed to allocate process memory\n");
        kfree(proc->stack_base);
        return NULL;
    }
    
    // Initialize CPU context
    memset(&proc->context, 0, sizeof(cpu_context_t));
    
    // Set up stack pointer (grows downward)
    proc->context.rsp = (uint64_t)proc->stack_base + stack_size - 8;
    proc->context.rbp = proc->context.rsp;
    
    // Set entry point
    proc->context.rip = (uint64_t)entry;
    
    // Set up arguments in RDI register (first argument for System V ABI)
    proc->context.rdi = (uint64_t)args;
    
    // Set default flags (interrupts enabled)
    proc->context.rflags = 0x202;
    
    // Use kernel page directory for now (no memory isolation yet)
    __asm__ volatile("mov %%cr3, %0" : "=r"(proc->context.cr3));
    
    // Add to process list
    process_add_to_list(proc);
    
    terminal_writestring("Created process: ");
    terminal_writestring(proc->name);
    terminal_writestring(" (PID: ");
    char pid_str[16];
    uint32_to_string(proc->pid, pid_str);
    terminal_writestring(pid_str);
    terminal_writestring(")\n");
    
    return proc;
}

// Terminate a process
void process_terminate(process_t* proc, int exit_code) {
    if (!proc) {
        return;
    }
    
    terminal_writestring("Terminating process: ");
    terminal_writestring(proc->name);
    terminal_writestring(" (PID: ");
    char pid_str[16];
    uint32_to_string(proc->pid, pid_str);
    terminal_writestring(pid_str);
    terminal_writestring(")\n");
    
    proc->state = PROCESS_STATE_TERMINATED;
    proc->exit_code = exit_code;
    
    // Free memory
    if (proc->stack_base) {
        kfree(proc->stack_base);
        proc->stack_base = NULL;
    }
    
    if (proc->memory_base) {
        kfree(proc->memory_base);
        proc->memory_base = NULL;
    }
    
    // Remove from process list
    process_remove_from_list(proc);
    
    // Don't automatically schedule if we're terminating the shell
    // or if it's not the current process
    if (current_process == proc) {
        // Check if this is the shell process
        if (strcmp(proc->name, "shell") == 0) {
            terminal_writestring("Shell terminated. System will halt.\n");
            current_process = NULL;
            __asm__ volatile("cli; hlt"); // Halt the system
        } else {
            current_process = NULL;
            // Only schedule if there are other processes to run
            if (process_list_head) {
                process_schedule();
            }
        }
    }
}

// Get the current running process
process_t* process_get_current(void) {
    return current_process;
}

// Get process by PID
process_t* process_get_by_pid(uint32_t pid) {
    if (!process_list_head) {
        return NULL;
    }
    
    process_t* proc = process_list_head;
    do {
        if (proc->pid == pid) {
            return proc;
        }
        proc = proc->next;
    } while (proc != process_list_head);
    
    return NULL;
}

// List all processes
void process_list(void) {
    terminal_writestring("=== Process List ===\n");
    
    if (!process_list_head) {
        terminal_writestring("No processes running.\n");
        return;
    }
    
    terminal_writestring("PID\tName\t\tState\t\tPriority\tMemory\n");
    terminal_writestring("---\t----\t\t-----\t\t--------\t------\n");
    
    process_t* proc = process_list_head;
    do {
        char pid_str[16];
        uint32_to_string(proc->pid, pid_str);
        terminal_writestring(pid_str);
        terminal_writestring("\t");
        terminal_writestring(proc->name);
        
        // Pad name to fixed width
        int name_len = strlen(proc->name);
        for (int i = name_len; i < 16; i++) {
            terminal_writestring(" ");
        }
        
        // State
        switch (proc->state) {
            case PROCESS_STATE_READY:
                terminal_writestring("READY\t\t");
                break;
            case PROCESS_STATE_RUNNING:
                terminal_writestring("RUNNING\t\t");
                break;
            case PROCESS_STATE_BLOCKED:
                terminal_writestring("BLOCKED\t\t");
                break;
            case PROCESS_STATE_TERMINATED:
                terminal_writestring("TERMINATED\t");
                break;
        }
        
        // Priority
        switch (proc->priority) {
            case PROCESS_PRIORITY_KERNEL:
                terminal_writestring("KERNEL\t\t");
                break;
            case PROCESS_PRIORITY_HIGH:
                terminal_writestring("HIGH\t\t");
                break;
            case PROCESS_PRIORITY_NORMAL:
                terminal_writestring("NORMAL\t\t");
                break;
            case PROCESS_PRIORITY_LOW:
                terminal_writestring("LOW\t\t");
                break;
        }
        
        // Memory usage
        char mem_str[16];
        uint32_to_string(proc->memory_size, mem_str);
        terminal_writestring(mem_str);
        terminal_writestring(" bytes\n");
        
        proc = proc->next;
    } while (proc != process_list_head);
}

// Find the next process to run
static process_t* process_find_next(void) {
    if (!process_list_head) {
        return NULL;
    }
    
    // Simple round-robin scheduling with priority
    process_t* best_proc = NULL;
    process_priority_t best_priority = PROCESS_PRIORITY_LOW + 1;
    
    process_t* proc = current_process ? current_process->next : process_list_head;
    process_t* start = proc;
    
    do {
        if (proc->state == PROCESS_STATE_READY && proc->priority < best_priority) {
            best_proc = proc;
            best_priority = proc->priority;
            
            // If we found a kernel priority process, use it immediately
            if (best_priority == PROCESS_PRIORITY_KERNEL) {
                break;
            }
        }
        proc = proc->next;
    } while (proc != start);
    
    return best_proc;
}

// Context switching function - simplified version
void process_switch_context(cpu_context_t* old_context, cpu_context_t* new_context) {
    // For now, we'll implement a simple cooperative multitasking
    // without full context switching. This is safer and easier to debug.
    
    if (old_context) {
        // Save minimal context
        __asm__ volatile("movq %%rsp, %0" : "=m"(old_context->rsp));
        __asm__ volatile("movq %%rbp, %0" : "=m"(old_context->rbp));
    }
    
    if (new_context) {
        // For new processes, we need to set up their initial state
        // For now, we'll just switch to their entry point directly
        // This is a simplified approach for initial implementation
        
        if (new_context->rip) {
            // This is a new process, call its entry point
            void (*entry_func)(void*) = (void (*)(void*))new_context->rip;
            void* args = (void*)new_context->rdi;
            
            // Call the process entry point
            entry_func(args);
            
            // If process returns, terminate it
            process_terminate(current_process, 0);
        }
    }
}

// Schedule the next process
void process_schedule(void) {
    if (!scheduler_initialized) {
        return;
    }
    
    process_t* next_proc = process_find_next();
    if (!next_proc) {
        // No ready processes, run kernel process or halt
        if (current_process && current_process->priority == PROCESS_PRIORITY_KERNEL) {
            return; // Already running kernel process
        }
        
        // Find kernel process
        process_t* kernel_proc = process_list_head;
        do {
            if (kernel_proc->priority == PROCESS_PRIORITY_KERNEL && 
                kernel_proc->state != PROCESS_STATE_TERMINATED) {
                next_proc = kernel_proc;
                break;
            }
            kernel_proc = kernel_proc->next;
        } while (kernel_proc != process_list_head);
        
        if (!next_proc) {
            terminal_writestring("No processes available, halting...\n");
            __asm__ volatile("hlt");
            return;
        }
    }
    
    if (next_proc == current_process) {
        // Same process, nothing to do
        return;
    }
    
    process_t* old_proc = current_process;
    
    // Update process states
    if (old_proc && old_proc->state == PROCESS_STATE_RUNNING) {
        old_proc->state = PROCESS_STATE_READY;
    }
    
    next_proc->state = PROCESS_STATE_RUNNING;
    current_process = next_proc;
    
    // For simplified implementation, directly call the process function
    // if it hasn't been started yet
    if (next_proc->context.rip) {
        void (*entry_func)(void*) = (void (*)(void*))next_proc->context.rip;
        void* args = (void*)next_proc->context.rdi;
        
        // Mark that this process has been started
        next_proc->context.rip = 0;
        
        // Call the process entry point
        entry_func(args);
        
        // If process returns, terminate it
        process_terminate(next_proc, 0);
    }
}

// Yield CPU to next process (simplified - no actual switching for now)
void process_yield(void) {
    // For now, just do a simple delay to prevent busy waiting
    // Real process switching will be implemented later
    for (volatile int i = 0; i < 1000; i++) {
        // Small delay
    }
}

// Sleep for specified milliseconds (simplified - no real timer)
void process_sleep(uint64_t milliseconds) {
    (void)milliseconds; // Ignore for now
    if (current_process) {
        current_process->state = PROCESS_STATE_BLOCKED;
        process_schedule();
    }
}

// Kernel process main function
void kernel_process_main(void* args) {
    (void)args;
    
    terminal_writestring("Kernel process started (PID: ");
    char pid_str[16];
    uint32_to_string(process_get_current()->pid, pid_str);
    terminal_writestring(pid_str);
    terminal_writestring(")\n");
    
    // The kernel process manages the system
    while (1) {
        // Basic scheduler tick - let other processes run
        process_t* current = process_get_current();
        if (current && current->priority != PROCESS_PRIORITY_KERNEL) {
            // If current process is not kernel priority, yield
            process_yield();
        }
        
        // Cleanup terminated processes
        process_cleanup_terminated();
        
        // Simple delay to prevent busy waiting
        for (volatile int i = 0; i < 10000; i++) {
            // Busy wait
        }
    }
}

// Clean up terminated processes
void process_cleanup_terminated(void) {
    if (!process_list_head) {
        return;
    }
    
    process_t* proc = process_list_head;
    process_t* next;
    
    do {
        next = proc->next;
        if (proc->state == PROCESS_STATE_TERMINATED) {
            process_remove_from_list(proc);
            // Process memory is already freed in process_terminate
        }
        proc = next;
    } while (proc != process_list_head && process_list_head);
}

// Shell process main function  
void shell_process_main(void* args); // Forward declaration only
