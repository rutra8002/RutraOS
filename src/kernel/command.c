#include "command.h"
#include "terminal.h"
#include "string.h"
#include "memory.h"
#include "fat16.h"

#define MAX_COMMANDS 64
#define MAX_COMMAND_SIZE 4096

// Command registry
static const command_info_t* command_registry[MAX_COMMANDS];
static size_t command_count = 0;

// Register a command in the registry
command_result_t command_register(const command_info_t* info) {
    if (!info || !info->name || !info->main) {
        return COMMAND_ERROR_INVALID_ARGS;
    }
    
    if (command_count >= MAX_COMMANDS) {
        return COMMAND_ERROR_LOAD_FAILED;
    }
    
    // Check if command already exists
    for (size_t i = 0; i < command_count; i++) {
        if (strcmp(command_registry[i]->name, info->name) == 0) {
            // Replace existing command
            command_registry[i] = info;
            return COMMAND_SUCCESS;
        }
    }
    
    // Add new command
    command_registry[command_count++] = info;
    return COMMAND_SUCCESS;
}

// Get command information by name
const command_info_t* command_get_info(const char* name) {
    if (!name) {
        return NULL;
    }
    
    for (size_t i = 0; i < command_count; i++) {
        if (strcmp(command_registry[i]->name, name) == 0) {
            return command_registry[i];
        }
    }
    
    return NULL;
}

// Execute a command by name
command_result_t command_execute(const char* name, int argc, char** argv) {
    if (!name) {
        return COMMAND_ERROR_INVALID_ARGS;
    }
    
    // First try to find in registry (built-in commands)
    const command_info_t* cmd = command_get_info(name);
    if (cmd) {
        int result = cmd->main(argc, argv);
        return (result == 0) ? COMMAND_SUCCESS : COMMAND_ERROR_EXEC_FAILED;
    }
    
    // Try to load from filesystem
    command_result_t load_result = command_load_from_file(name);
    if (load_result == COMMAND_SUCCESS) {
        // Try to execute the loaded command
        cmd = command_get_info(name);
        if (cmd) {
            int result = cmd->main(argc, argv);
            return (result == 0) ? COMMAND_SUCCESS : COMMAND_ERROR_EXEC_FAILED;
        }
    }
    
    return COMMAND_ERROR_NOT_FOUND;
}

// Load command from file (simplified - in real OS this would load ELF/PE files)
command_result_t command_load_from_file(const char* filename) {

    
    char file_buffer[MAX_COMMAND_SIZE];
    int size = fat16_read_file(filename, file_buffer, sizeof(file_buffer) - 1);
    
    if (size <= 0) {
        return COMMAND_ERROR_NOT_FOUND;
    }
    
    terminal_writestring("Command loading from files not fully implemented yet\n");
    return COMMAND_ERROR_LOAD_FAILED;
}

// List all available commands
void command_list_available(void) {
    terminal_writestring("Available commands:\n");
    
    for (size_t i = 0; i < command_count; i++) {
        terminal_writestring("  ");
        terminal_writestring(command_registry[i]->name);
        if (command_registry[i]->description) {
            terminal_writestring(" - ");
            terminal_writestring(command_registry[i]->description);
        }
        terminal_writestring("\n");
    }
}

// Utility function to parse command line into argc/argv
int command_parse_args(const char* input, char*** argv_out) {
    if (!input || !argv_out) {
        return 0;
    }
    
    // Count arguments
    int argc = 0;
    const char* p = input;
    int in_word = 0;
    
    while (*p) {
        if (*p != ' ' && *p != '\t') {
            if (!in_word) {
                argc++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
        p++;
    }
    
    if (argc == 0) {
        *argv_out = NULL;
        return 0;
    }
    
    // Allocate argv array
    char** argv = (char**)kmalloc((argc + 1) * sizeof(char*));
    if (!argv) {
        *argv_out = NULL;
        return 0;
    }
    
    // Parse arguments
    int arg_index = 0;
    p = input;
    in_word = 0;
    const char* word_start = NULL;
    
    while (*p || in_word) {
        if ((*p != ' ' && *p != '\t' && *p != '\0') && !in_word) {
            // Start of new word
            word_start = p;
            in_word = 1;
        } else if ((*p == ' ' || *p == '\t' || *p == '\0') && in_word) {
            // End of word
            size_t word_len = p - word_start;
            argv[arg_index] = (char*)kmalloc(word_len + 1);
            if (argv[arg_index]) {
                strncpy(argv[arg_index], word_start, word_len);
                argv[arg_index][word_len] = '\0';
                arg_index++;
            }
            in_word = 0;
        }
        
        if (*p == '\0') break;
        p++;
    }
    
    argv[arg_index] = NULL;
    *argv_out = argv;
    return argc;
}

// Free parsed arguments
void command_free_args(int argc, char** argv) {
    if (!argv) return;
    
    for (int i = 0; i < argc; i++) {
        if (argv[i]) {
            kfree(argv[i]);
        }
    }
    kfree(argv);
}

// Check if --help flag is present in arguments
int command_check_help_flag(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            return 1;
        }
    }
    return 0;
}

// Show command usage information
void command_show_usage(const char* command_name, const char* usage_text) {
    terminal_writestring("Usage: ");
    terminal_writestring(command_name);
    terminal_writestring(" ");
    terminal_writestring(usage_text);
    terminal_writestring("\n");
}

// External command info declarations
extern const command_info_t cmd_info_cmd_help_main;
extern const command_info_t cmd_info_cmd_clear_main;
extern const command_info_t cmd_info_cmd_meminfo_main;
extern const command_info_t cmd_info_cmd_memtest_main;
extern const command_info_t cmd_info_cmd_ls_main;
extern const command_info_t cmd_info_cmd_cat_main;
extern const command_info_t cmd_info_cmd_create_main;
extern const command_info_t cmd_info_cmd_ps_main;
extern const command_info_t cmd_info_cmd_kill_main;
extern const command_info_t cmd_info_cmd_spawn_main;
extern const command_info_t cmd_info_cmd_shutdown_main;
extern const command_info_t cmd_info_cmd_reboot_main;
extern const command_info_t cmd_info_cmd_ramdisk_main;
extern const command_info_t cmd_info_cmd_mkdir_main;
extern const command_info_t cmd_info_cmd_cd_main;
extern const command_info_t cmd_info_cmd_pwd_main;
extern const command_info_t cmd_info_cmd_gfx_main;
extern const command_info_t cmd_info_cmd_draw_main;
extern const command_info_t cmd_info_cmd_fontdemo_main;

// Register all built-in commands
void command_register_builtins(void) {
    command_register(&cmd_info_cmd_help_main);
    command_register(&cmd_info_cmd_clear_main);
    command_register(&cmd_info_cmd_meminfo_main);
    command_register(&cmd_info_cmd_memtest_main);
    command_register(&cmd_info_cmd_ls_main);
    command_register(&cmd_info_cmd_cat_main);
    command_register(&cmd_info_cmd_create_main);
    command_register(&cmd_info_cmd_ps_main);
    command_register(&cmd_info_cmd_kill_main);
    command_register(&cmd_info_cmd_spawn_main);
    command_register(&cmd_info_cmd_shutdown_main);
    command_register(&cmd_info_cmd_reboot_main);
    command_register(&cmd_info_cmd_ramdisk_main);
    command_register(&cmd_info_cmd_mkdir_main);
    command_register(&cmd_info_cmd_cd_main);
    command_register(&cmd_info_cmd_pwd_main);
    command_register(&cmd_info_cmd_gfx_main);
    command_register(&cmd_info_cmd_draw_main);
    command_register(&cmd_info_cmd_fontdemo_main);
}
