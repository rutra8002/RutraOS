#ifndef SHELL_H
#define SHELL_H

// System control functions
void shell_shutdown();
void shell_reboot();

// Process-based shell functions
void shell_process_main(void* args);

#endif
