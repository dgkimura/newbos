/*
This is a function to load the gdt.
*/
.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    mov 4(%esp), %eax
    lgdt (%eax)
    jmp $0x08, $flush

flush:
    # load the kernel data segment descriptor
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ret
