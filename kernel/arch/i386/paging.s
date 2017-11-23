.section .text
.align 4

.global enable_paging
.type enable_paging, @function
enable_paging:
    mov 4(%esp), %eax
    mov %eax, %cr3

    mov %cr0, %eax
    or  $0x80000000, %eax
    mov %eax, %cr0
    ret

.global get_fault_address
.type get_fault_address, @function
get_fault_address:
    mov %cr2, %eax
    ret

.global copy_page_physical
.type copy_page_physical, @function
copy_page_physical:
    # According to __cdecl, we must preserve the contents of EBX
    push %ebx

    # Push EFLAGS, so we can pop it and reenable interrupts later, if they were
    # enabled anyway.
    pushf

    # Disable interrupts so we aren't interrupted
    cli

    # source address
    mov 12(%esp), %ebx

    # destination address
    mov 16(%esp), %ecx

    # disable paging
    mov %cr0, %edx
    and $0x7fffffff, %edx
    mov %edx, %cr0

    mov $1024, %edx

.loop:
    mov (%ebx), %eax
    mov %eax, (%ecx)
    add $4, %ebx
    add $4, %ecx
    dec %edx
    jnz .loop

    mov %cr0, %edx
    or $0x80000000, %edx
    mov %edx, %cr0

    popf
    pop %ebx
    ret
