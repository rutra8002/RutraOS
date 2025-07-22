#ifndef COMMAND_H
#define COMMAND_H

#include "types.h"

// Command execution context
typedef struct {
    int argc;                    // Number of arguments
    char** argv;                 // Argument array
    void* stdin_func;            // Function to read input
    void* stdout_func;           // Function to write output
    void* stderr_func;           // Function to write errors
} command_context_t;

// Command entry point function type
typedef int (*command_main_t)(int argc, char** argv);

// Command metadata structure
typedef struct {
    const char* name;            // Command name
    const char* description;     // Command description
    command_main_t main;         // Entry point function
    uint32_t version;            // Command version
} command_info_t;

// Command execution result
typedef enum {
    COMMAND_SUCCESS = 0,
    COMMAND_ERROR_NOT_FOUND = -1,
    COMMAND_ERROR_LOAD_FAILED = -2,
    COMMAND_ERROR_EXEC_FAILED = -3,
    COMMAND_ERROR_INVALID_ARGS = -4
} command_result_t;

// Command loader functions
command_result_t command_execute(const char* name, int argc, char** argv);
command_result_t command_load_from_file(const char* filename);
command_result_t command_register(const command_info_t* info);
void command_list_available(void);
const command_info_t* command_get_info(const char* name);

// Command line parsing utilities
int command_parse_args(const char* input, char*** argv_out);
void command_free_args(int argc, char** argv);

// Command help utilities
int command_check_help_flag(int argc, char** argv);
void command_show_usage(const char* command_name, const char* usage_text);

// Built-in command registration macro
#define REGISTER_COMMAND(name, desc, func) \
    const command_info_t cmd_info_##func = { \
        name, \
        desc, \
        func, \
        1 \
    };

// Command registration helper - call this to register all built-in commands
void command_register_builtins(void);

#endif // COMMAND_H
