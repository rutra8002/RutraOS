#include "command.h"
#include "terminal.h"
#include "process.h"
#include "string.h"

static int cmd_kill_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("kill", "<pid> [confirm]");
        terminal_writestring("Terminate a process by its PID.\n");
        terminal_writestring("Use 'confirm' to kill system processes without warning.\n");
        return 0;
    }
    
    if (argc < 2) {
        terminal_writestring("Usage: kill <pid> [confirm]\n");
        return 1;
    }
    
    int confirm = 0;
    if (argc >= 3 && strcmp(argv[2], "confirm") == 0) {
        confirm = 1;
    }
    
    // Simple string to int conversion
    uint32_t pid = (uint32_t)atoi(argv[1]);
    
    if (pid == 0) {
        terminal_writestring("Invalid PID\n");
        return 1;
    }
    
    process_t* proc = process_get_by_pid(pid);
    if (!proc) {
        terminal_writestring("Process not found\n");
        return 1;
    }
    
    if (proc->priority == PROCESS_PRIORITY_KERNEL) {
        terminal_writestring("Cannot kill kernel process\n");
        return 1;
    }
    
    // Warn about killing the shell
    if (strcmp(proc->name, "shell") == 0 && !confirm) {
        terminal_writestring("Warning: Killing shell will halt the system!\n");
        terminal_writestring("Type 'kill ");
        char pid_str[16];
        uint32_to_string(pid, pid_str);
        terminal_writestring(pid_str);
        terminal_writestring(" confirm' to proceed.\n");
        return 1;
    }
    
    process_terminate(proc, -1);
    terminal_writestring("Process terminated\n");
    return 0;
}

REGISTER_COMMAND("kill", "Terminate a process", cmd_kill_main)
