.section .text
.align 4

.global get_ebp
.type get_ebp, @function
get_ebp:
    mov %ebp, %eax
    ret

.global get_esp
.type get_esp, @function
get_esp:
    mov %esp, %eax
    ret

.global get_eip
.type get_eip, @function
get_eip:
    pop %eax
    ret

.global set_ebp
.type set_ebp, @function
set_ebp:
    mov 4(%esp), %eax
    mov %eax, %ebp
    ret

.global set_esp
.type set_esp, @function
set_esp:
    mov 4(%esp), %eax
    mov %eax, %esp
    ret
