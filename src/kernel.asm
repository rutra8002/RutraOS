[BITS 16]           ; We're still in 16-bit mode when loaded
[ORG 0x1000]        ; Kernel will be loaded at 0x1000

kernel_start:
    ; Clear screen
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    
    ; Print kernel message
    mov si, kernel_msg
    call print_string
    
    ; Infinite loop
    jmp $

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

kernel_msg db 'Kernel loaded successfully!', 0x0D, 0x0A, 0

; Pad kernel to sector boundary (512 bytes)
times 512-($-$$) db 0
