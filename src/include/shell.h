#ifndef SHELL_H
#define SHELL_H

// Shell functions (legacy - for backward compatibility)
void shell_init();
void shell_handle_input(char c);
void shell_shutdown();
void shell_reboot();

// Process-based shell functions
void shell_process_main(void* args);

#endif
