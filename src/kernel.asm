[BITS 16]           ; We're still in 16-bit mode when loaded
[ORG 0x1000]        ; Kernel will be loaded at 0x1000

kernel_start:
    ; Clear screen
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    
    ; Print welcome message
    mov si, welcome_msg
    call print_string
    
    ; Add a small delay to see if kernel is running
    mov cx, 0xFFFF
.delay:
    dec cx
    jnz .delay
    
    ; Start shell loop
    jmp shell_loop

shell_loop:
    ; Print prompt
    mov si, prompt
    call print_string
    
    ; Read command
    call read_input
    
    ; Process command
    call process_command
    
    ; Repeat
    jmp shell_loop

read_input:
    mov di, input_buffer    ; Buffer to store input
    xor cx, cx             ; Counter for input length
    
.read_char:
    ; Get character from keyboard
    mov ah, 0x00
    int 0x16               ; BIOS keyboard interrupt
    
    ; Check for Enter key
    cmp al, 0x0D
    je .input_done
    
    ; Check for Backspace
    cmp al, 0x08
    je .handle_backspace
    
    ; Check if buffer is full
    cmp cx, 63
    jge .read_char
    
    ; Store character and echo it
    mov [di], al
    inc di
    inc cx
    
    ; Echo character to screen
    mov ah, 0x0E
    int 0x10
    
    jmp .read_char

.handle_backspace:
    ; Check if there's something to delete
    cmp cx, 0
    je .read_char
    
    ; Move cursor back and erase character
    dec di
    dec cx
    
    ; Print backspace, space, backspace
    mov ah, 0x0E
    mov al, 0x08
    int 0x10
    mov al, ' '
    int 0x10
    mov al, 0x08
    int 0x10
    
    jmp .read_char

.input_done:
    ; Null-terminate the string
    mov byte [di], 0
    
    ; Print newline
    call print_newline
    ret

process_command:
    ; Check if input is empty
    mov si, input_buffer
    mov al, [si]
    cmp al, 0
    je .done
    
    ; Debug: print what was entered
    mov si, debug_msg
    call print_string
    mov si, input_buffer
    call print_string
    call print_newline
    
    ; Compare with "help" command
    mov si, input_buffer
    mov di, cmd_help
    call compare_strings
    cmp ax, 1
    je .cmd_help
    
    ; Compare with "clear" command
    mov si, input_buffer
    mov di, cmd_clear
    call compare_strings
    cmp ax, 1
    je .cmd_clear
    
    ; Compare with "about" command
    mov si, input_buffer
    mov di, cmd_about
    call compare_strings
    cmp ax, 1
    je .cmd_about
    
    ; Compare with "halt" command
    mov si, input_buffer
    mov di, cmd_halt
    call compare_strings
    cmp ax, 1
    je .cmd_halt
    
    ; Unknown command
    mov si, unknown_cmd_msg
    call print_string
    jmp .done

.cmd_help:
    mov si, help_msg
    call print_string
    jmp .done

.cmd_clear:
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    jmp .done

.cmd_about:
    mov si, about_msg
    call print_string
    jmp .done

.cmd_halt:
    mov si, halt_msg
    call print_string
    cli
    hlt

.done:
    ret

compare_strings:
    ; Compare strings at SI and DI
    ; Returns AX = 1 if equal, 0 if different
.loop:
    mov al, [si]
    mov bl, [di]
    cmp al, bl
    jne .not_equal
    
    cmp al, 0
    je .equal
    
    inc si
    inc di
    jmp .loop

.equal:
    mov ax, 1
    ret

.not_equal:
    mov ax, 0
    ret

print_string:
    mov ah, 0x0E        ; BIOS teletype function
.loop:
    lodsb               ; Load next character
    cmp al, 0           ; Check for null terminator
    je .done
    int 0x10            ; Print character
    jmp .loop
.done:
    ret

print_newline:
    mov ah, 0x0E
    mov al, 0x0D        ; Carriage return
    int 0x10
    mov al, 0x0A        ; Line feed
    int 0x10
    ret

; Data section
welcome_msg db 'RutraOS v0.1', 0x0D, 0x0A, 'Type "help" for available commands.', 0x0D, 0x0A, 0x0A, 0
prompt db '> ', 0
help_msg db 'Available commands:', 0x0D, 0x0A
         db '  help  - Show this help message', 0x0D, 0x0A
         db '  clear - Clear the screen', 0x0D, 0x0A
         db '  about - Show system information', 0x0D, 0x0A
         db '  halt  - Halt the system', 0x0D, 0x0A, 0
about_msg db 'RutraOS v0.1', 0x0D, 0x0A
          db 'A basic operating system with shell', 0x0D, 0x0A
          db 'Built with NASM assembly', 0x0D, 0x0A, 0
halt_msg db 'System halted. You can now power off.', 0x0D, 0x0A, 0
unknown_cmd_msg db 'Unknown command. Type "help" for available commands.', 0x0D, 0x0A, 0
debug_msg db 'You entered: ', 0

; Command strings
cmd_help db 'help', 0
cmd_clear db 'clear', 0
cmd_about db 'about', 0
cmd_halt db 'halt', 0

; Input buffer
input_buffer times 64 db 0

; Pad kernel to multiple of 512 bytes
times 1024-($-$$) db 0
