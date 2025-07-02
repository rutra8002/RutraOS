; Multiboot header constants
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_FLAGS     equ 0x00000003
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

[BITS 32]           
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
    
    ; Print kernel message
    mov esi, kernel_msg
    call print_string
    
    ; Halt system
    cli
    hlt

print_string:
    mov edi, 0xB8000    ; VGA text buffer
    mov ah, 0x0F        ; White on black attribute
.loop:
    lodsb               ; Load next character
    test al, al         ; Check for null terminator
    jz .done
    stosw               ; Store character and attribute
    jmp .loop
.done:
    ret

kernel_msg db 'RutraOS Kernel Started!', 0

section .bss
    resb 8192       ; 8KB stack
stack_top:
