#include "command.h"
#include "terminal.h"
#include "process.h"
#include "string.h"

// Test process function
static void test_process_main(void* args) {
    // Use the name from the process structure as args might be invalid if passed from stack
    process_t* current = process_get_current();
    const char* name = current->name;
    
    terminal_writestring("Test process ");
    terminal_writestring(name);
    terminal_writestring(" started\n");
    
    // Do some work
    for (int i = 0; i < 5; i++) {
        terminal_writestring(name);
        terminal_writestring(" working... iteration ");
        char iter_str[16];
        uint32_to_string(i + 1, iter_str);
        terminal_writestring(iter_str);
        terminal_writestring("\n");
        
        // Yield to let other processes run
        process_yield();
        
        // Small delay
        for (volatile int j = 0; j < 1000000; j++) {
            // Busy wait
        }
    }
    
    terminal_writestring("Test process ");
    terminal_writestring(name);
    terminal_writestring(" finished\n");
    
    // Process will be terminated automatically when this function returns
}

static int cmd_spawn_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("spawn", "<name>");
        terminal_writestring("Create a new test process with the given name.\n");
        terminal_writestring("The process will run in the background.\n");
        return 0;
    }
    
    if (argc < 2) {
        terminal_writestring("Usage: spawn <name>\n");
        return 1;
    }
    
    // Create a name for the test process
    static char process_name[64];
    strncpy(process_name, argv[1], sizeof(process_name) - 1);
    process_name[sizeof(process_name) - 1] = '\0';
    
    process_t* proc = process_create(process_name, test_process_main, NULL,
                                   PROCESS_PRIORITY_NORMAL, 8192);
    if (proc) {
        terminal_writestring("Created process: ");
        terminal_writestring(process_name);
        terminal_writestring(" (PID: ");
        char pid_str[16];
        uint32_to_string(proc->pid, pid_str);
        terminal_writestring(pid_str);
        terminal_writestring(")\n");
        
        terminal_writestring("Process spawned in background.\n");
        return 0;
    } else {
        terminal_writestring("Failed to spawn process\n");
        return 1;
    }
}

REGISTER_COMMAND("spawn", "Create a new test process", cmd_spawn_main)
