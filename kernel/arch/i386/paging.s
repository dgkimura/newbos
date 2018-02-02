.section .text
.align 4

.global pagetable_load
.type pagetable_load, @function
pagetable_load:
    mov 4(%esp), %eax
    mov %eax, %cr3
    ret

.global pagetable_enable
.type pagetable_enable, @function
pagetable_enable:
    mov %cr0, %eax
    or  $0x80000000, %eax
    mov %eax, %cr0
    ret
