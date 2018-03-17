.section .text
.align 4

.global pagetable_load
.type pagetable_load, @function
pagetable_load:
    mov 4(%esp), %eax
    mov %eax, %cr3
    ret

.global invalidate_page_table_entry
.type invalidate_page_table_entry, @function
invalidate_page_table_entry:
    mov 4(%esp), %eax
    invlpg (%eax)
    ret
