
.global run_process_in_user_mode
.type run_process_in_user_mode, @function
run_process_in_user_mode:
    cli                             # disable external interrupts
    mov     4(%esp), %eax           # load address of registers_t into eax

    # restore all the registers except eax
    mov     4(%eax), %ebx
    mov     8(%eax), %ecx
    mov    12(%eax), %edx
    mov    16(%eax), %ebp
    mov    20(%eax), %esi
    mov    24(%eax), %edi

    # push information for iret onto the stack
    push   28(%eax)          # push the SS onto the stack
    push   32(%eax)          # push the ESP of the user stack
    push   36(%eax)          # push EFLAGS
    push   40(%eax)          # push the segment selector
    push   44(%eax)          # push EIP

    # move index for the data segment into data registers
    mov     $0x18, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs

    mov     (%eax), %eax              # restore eax

    # FIXME: Use iret to pop off SS, ESP, EFLAGS, SS, and EIP
    #iret                            # iret into the given mode
    pop     %eax
    pop     %eax
    pop     %eax
    pop     %eax
    pop     %eax
    ret
