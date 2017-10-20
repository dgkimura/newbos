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
