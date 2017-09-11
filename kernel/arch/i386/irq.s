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

.global irq1
.type irq1, @function
irq1:
    cli
    push $0
    push $33
    jmp irq_common_stub

.global irq2
.type irq2, @function
irq2:
    cli
    push $0
    push $34
    jmp irq_common_stub

.global irq3
.type irq3, @function
irq3:
    cli
    push $0
    push $35
    jmp irq_common_stub

.global irq4
.type irq4, @function
irq4:
    cli
    push $0
    push $36
    jmp irq_common_stub

.global irq5
.type irq5, @function
irq5:
    cli
    push $0
    push $37
    jmp irq_common_stub

.global irq6
.type irq6, @function
irq6:
    cli
    push $0
    push $38
    jmp irq_common_stub

.global irq7
.type irq7, @function
irq7:
    cli
    push $0
    push $39
    jmp irq_common_stub

.global irq8
.type irq8, @function
irq8:
    cli
    push $0
    push $40
    jmp irq_common_stub

.global irq9
.type irq9, @function
irq9:
    cli
    push $0
    push $41
    jmp irq_common_stub

.global irq10
.type irq10, @function
irq10:
    cli
    push $0
    push $42
    jmp irq_common_stub

.global irq11
.type irq11, @function
irq11:
    cli
    push $0
    push $43
    jmp irq_common_stub

.global irq12
.type irq12, @function
irq12:
    cli
    push $0
    push $44
    jmp irq_common_stub

.global irq13
.type irq13, @function
irq13:
    cli
    push $0
    push $45
    jmp irq_common_stub

.global irq14
.type irq14, @function
irq14:
    cli
    push $0
    push $46
    jmp irq_common_stub

.global irq15
.type irq15, @function
irq15:
    cli
    push $0
    push $47
    jmp irq_common_stub

irq_common_stub:
    pusha

    # save data segment
    push    %ds
    push    %es
    push    %fs
    push    %gs

    # load the kernel data segment descriptor
    mov     $0x10, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs

    mov     %esp, %eax
    push    %eax

    call    irq_handler

    pop     %eax

    # restore data segment
    pop     %gs
    pop     %fs
    pop     %es
    pop     %ds

    popa

    addl    $4, %esp
    addl    $4, %esp
    sti
    iret
