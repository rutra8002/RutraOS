; Multiboot header constants
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_FLAGS     equ 0x00000003
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

[BITS 32]           ; We're now in 32-bit protected mode
[GLOBAL _start]     ; Make _start globally visible

section .multiboot
    ; Multiboot header
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .text
_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Clear screen (VGA text mode)
    call clear_screen
    
    ; Print kernel message
    mov esi, kernel_msg
    call print_string
    
    ; Infinite loop
    cli
    hlt
.hang:
    jmp .hang

clear_screen:
    pusha
    mov edi, 0xB8000    ; VGA text buffer
    mov ecx, 80*25      ; 80 columns * 25 rows
    mov ax, 0x0720      ; Space character with gray on black
    rep stosw
    popa
    ret

print_string:
    pusha
    mov edi, 0xB8000    ; VGA text buffer
    mov ah, 0x07        ; Gray on black attribute
.loop:
    lodsb               ; Load next character
    cmp al, 0           ; Check for null terminator
    je .done
    cmp al, 0x0A        ; Check for newline
    je .newline
    cmp al, 0x0D        ; Check for carriage return
    je .loop            ; Skip carriage return
    stosw               ; Store character and attribute
    jmp .loop
.newline:
    ; Calculate new line position
    push eax
    mov eax, edi
    sub eax, 0xB8000
    shr eax, 1          ; Divide by 2 (each char is 2 bytes)
    mov ebx, 80
    div ebx             ; Get current row
    inc eax             ; Move to next row
    mul ebx             ; Multiply by 80 to get position
    shl eax, 1          ; Multiply by 2 (each char is 2 bytes)
    add eax, 0xB8000
    mov edi, eax
    pop eax
    jmp .loop
.done:
    popa
    ret

kernel_msg db 'Kernel loaded successfully with GRUB!', 0x0A, 0x0A, 'Welcome to RutraOS!', 0

section .bss
    resb 8192       ; 8KB stack
stack_top:
