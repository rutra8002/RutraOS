; Multiboot header constants
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_FLAGS     equ 0x00000003
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

[BITS 32]           
[GLOBAL _start]     ; Make _start globally visible
[EXTERN kernel_main] ; Declare external C function

section .multiboot
    ; Multiboot header
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .text
_start:
    ; Disable interrupts
    cli
    
    ; Set up stack
    mov esp, stack_top
    
    ; Check if CPUID is supported
    call check_cpuid
    call check_long_mode
    
    ; Set up paging for long mode
    call setup_page_tables
    call enable_paging
    
    ; Enable SSE
    call enable_sse
    
    ; Load GDT and jump to 64-bit code
    lgdt [gdt64.pointer]
    jmp gdt64.code:long_mode_start

check_cpuid:
    ; Check if CPUID is supported by attempting to flip ID bit in EFLAGS
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    jz .no_cpuid
    ret
.no_cpuid:
    mov al, "1"
    jmp error

check_long_mode:
    ; Test if extended processor info is available
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    
    ; Use extended info to test if long mode is available
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    ret
.no_long_mode:
    mov al, "2"
    jmp error

setup_page_tables:
    ; Map first P4 entry to P3 table
    mov eax, p3_table
    or eax, 0b11    ; present + writable
    mov [p4_table], eax
    
    ; Map first P3 entry to P2 table
    mov eax, p2_table
    or eax, 0b11    ; present + writable
    mov [p3_table], eax
    
    ; Map each P2 entry to a huge 2MB page
    mov ecx, 0
.map_p2_table:
    mov eax, 0x200000   ; 2MB
    mul ecx
    or eax, 0b10000011  ; present + writable + huge
    mov [p2_table + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jne .map_p2_table
    
    ret

enable_paging:
    ; Load P4 to cr3 register
    mov eax, p4_table
    mov cr3, eax
    
    ; Enable PAE flag in cr4
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    
    ; Set long mode bit in EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    
    ; Enable paging in cr0
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    
    ret

enable_sse:
    ; Check for SSE support
    mov eax, 1
    cpuid
    test edx, 1<<25
    jz .no_sse

    ; Enable SSE
    mov eax, cr0
    and ax, 0xFFFB      ; Clear EM
    or ax, 0x2          ; Set MP
    mov cr0, eax
    
    mov eax, cr4
    or ax, 3<<9         ; Set OSFXSR and OSXMMEXCPT
    mov cr4, eax
    
    ret
.no_sse:
    mov al, "a"
    jmp error

error:
    ; Print "ERR: X" where X is the error code
    mov dword [0xB8000], 0x4F524F45
    mov dword [0xB8004], 0x4F3A4F52
    mov dword [0xB8008], 0x4F204F20
    mov byte [0xB800A], al
    hlt

[BITS 64]
long_mode_start:
    ; Load 0 into all data segment registers
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Set up the stack pointer for C code
    mov rsp, stack_top
    
    ; Call the C kernel main function
    call kernel_main
    
    ; If kernel_main returns, halt the system
    cli
    hlt

; Process context switching function
; void process_switch_context(cpu_context_t* old_context, cpu_context_t* new_context);
; rdi = old_context, rsi = new_context
global process_switch_context
process_switch_context:
    ; If old_context is NULL, skip saving
    test rdi, rdi
    jz .load_new
    
    ; Save general purpose registers
    mov [rdi + 0], rax
    mov [rdi + 8], rbx
    mov [rdi + 16], rcx
    mov [rdi + 24], rdx
    mov [rdi + 32], rsi
    mov [rdi + 40], rdi
    mov [rdi + 48], rbp
    
    ; Save stack pointer (rsp)
    ; The rsp currently points to the return address pushed by call
    ; We want the rsp value BEFORE the call + 8 (to skip return address)
    lea rax, [rsp + 8]
    mov [rdi + 56], rax
    
    mov [rdi + 64], r8
    mov [rdi + 72], r9
    mov [rdi + 80], r10
    mov [rdi + 88], r11
    mov [rdi + 96], r12
    mov [rdi + 104], r13
    mov [rdi + 112], r14
    mov [rdi + 120], r15
    
    ; Save instruction pointer (rip)
    ; The return address is at [rsp]
    mov rax, [rsp]
    mov [rdi + 128], rax
    
    ; Save RFLAGS
    pushfq
    pop rax
    mov [rdi + 136], rax
    
    ; Save CR3
    mov rax, cr3
    mov [rdi + 144], rax

.load_new:
    ; Load new context from rsi
    
    ; Restore CR3 first (if we were changing address spaces)
    mov rax, [rsi + 144]
    mov cr3, rax
    
    ; Restore RFLAGS
    mov rax, [rsi + 136]
    push rax
    popfq
    
    ; Restore general purpose registers
    mov rax, [rsi + 0]
    mov rbx, [rsi + 8]
    mov rcx, [rsi + 16]
    mov rdx, [rsi + 24]
    ; rsi restored last
    mov rdi, [rsi + 40]
    mov rbp, [rsi + 48]
    mov rsp, [rsi + 56]
    
    mov r8,  [rsi + 64]
    mov r9,  [rsi + 72]
    mov r10, [rsi + 80]
    mov r11, [rsi + 88]
    mov r12, [rsi + 96]
    mov r13, [rsi + 104]
    mov r14, [rsi + 112]
    mov r15, [rsi + 120]
    
    ; Push RIP onto the stack so ret will jump to it
    mov rax, [rsi + 128]
    push rax
    
    ; Restore RSI
    mov rsi, [rsi + 32]
    
    ; Jump to the new RIP
    ret

section .rodata
gdt64:
    dq 0 ; zero entry
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; code segment
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

section .bss
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096
    
align 16
    resb 16384      ; 16KB stack
stack_top:
