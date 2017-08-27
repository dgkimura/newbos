/*
This macro creates a stub for an IRQ - the first parameter is
the IRQ number, the second is the ISR number it is remapped to
*/
.global irq0
.type irq0, @function
irq0:
    cli
    push $0
    push $32
    jmp irq_common_stub

irq_common_stub:
    pushl   %ds
    pushl   %eax
    pushl   %ecx
    pushl   %edx
    pushl   %ebx
    pushl   %esp
    pushl   %ebp
    pushl   %esi

    # push interrupt code and interrupt number
    pushl   36(%esp)
    pushl   36(%esp)

    # load the kernel data segment descriptor
    mov     $0x10, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs
    mov     %ax, %ss

    call    irq_handler

    addl    $4, %esp
    addl    $4, %esp

    popl    %esi
    popl    %ebp
    popl    %esp
    popl    %ebx
    popl    %edx
    popl    %ecx
    popl    %eax
    popl    %ds
    addl    $4, %esp
    addl    $4, %esp
    iret
