/*
 * Load the gdt into the cpu, and enter the kernel segments.
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

.global tss_load_and_set
.type tss_load_and_set, @function
tss_load_and_set:
    mov 4(%esp), %ax     # mov the tss segsel into ax (16 bits)
    ltr %ax              # load the task register with the selector
    ret
