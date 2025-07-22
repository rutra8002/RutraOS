#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

// Process states
typedef enum {
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_TERMINATED
} process_state_t;

// Process priorities
typedef enum {
    PROCESS_PRIORITY_KERNEL = 0,
    PROCESS_PRIORITY_HIGH = 1,
    PROCESS_PRIORITY_NORMAL = 2,
    PROCESS_PRIORITY_LOW = 3
} process_priority_t;

// CPU context structure
typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t rip;
    uint64_t rflags;
    uint64_t cr3;  // Page directory
} cpu_context_t;

// Process Control Block (PCB)
typedef struct process {
    uint32_t pid;                    // Process ID
    char name[64];                   // Process name
    process_state_t state;           // Current state
    process_priority_t priority;     // Process priority
    
    cpu_context_t context;           // CPU context
    void* stack_base;                // Stack base address
    size_t stack_size;               // Stack size
    void* memory_base;               // Process memory base
    size_t memory_size;              // Process memory size
    
    struct process* parent;          // Parent process
    struct process* next;            // Next process in list
    struct process* prev;            // Previous process in list
    
    uint64_t time_slice;             // Time slice for round-robin
    uint64_t time_used;              // Time used in current slice
    uint64_t total_time;             // Total CPU time used
    
    int exit_code;                   // Exit code when terminated
} process_t;

// Process function type
typedef void (*process_entry_t)(void* args);

// Process management functions
void process_init(void);
process_t* process_create(const char* name, process_entry_t entry, void* args, 
                         process_priority_t priority, size_t stack_size);
void process_terminate(process_t* proc, int exit_code);
void process_yield(void);
void process_schedule(void);
void process_sleep(uint64_t milliseconds);

// Current process access
extern process_t* current_process;
extern process_t* process_list_head;

// Process information functions
process_t* process_get_current(void);
process_t* process_get_by_pid(uint32_t pid);
void process_list(void);

// Context switching functions
void process_switch_context(cpu_context_t* old_context, cpu_context_t* new_context);
void process_save_context(cpu_context_t* context);
void process_load_context(cpu_context_t* context);

// Kernel process functions
void kernel_process_main(void* args);
void shell_process_main(void* args);

// Process utilities
uint32_t process_generate_pid(void);
void process_cleanup_terminated(void);

#endif // PROCESS_H
