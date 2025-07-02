[BITS 16]           ; 16-bit mode
[ORG 0x7C00]        ; BIOS loads boot sector at 0x7C00

boot_start:
    ; Set up segments
    cli                 ; Clear interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00      ; Set stack pointer
    sti                 ; Enable interrupts
    
    ; Print boot message
    mov si, boot_msg
    call print_string
    
    ; Load kernel from disk
    mov ah, 0x02        ; BIOS read sector function
    mov al, 1           ; Number of sectors to read
    mov ch, 0           ; Cylinder number
    mov cl, 2           ; Sector number (sector 2, after boot sector)
    mov dh, 0           ; Head number
    mov dl, 0x80        ; Drive number (0x80 = first hard disk)
    mov bx, 0x1000      ; Load kernel at 0x1000
    int 0x13            ; BIOS disk interrupt
    
    jc disk_error       ; Jump if carry flag set (error)
    
    ; Jump to kernel
    mov si, jump_msg
    call print_string
    jmp 0x1000          ; Jump to kernel
    
disk_error:
    mov si, error_msg
    call print_string
    jmp $               ; Infinite loop on error

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

boot_msg db 'Bootloader started...', 0x0D, 0x0A, 0
jump_msg db 'Jumping to kernel...', 0x0D, 0x0A, 0
error_msg db 'Disk read error!', 0x0D, 0x0A, 0

times 510-($-$$) db 0   ; Fill rest of sector with zeros
db 0x55, 0xAA           ; Boot signature