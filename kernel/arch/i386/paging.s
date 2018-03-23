.section .text
.align 4

.global pdt_set
.type pdt_set, @function
pdt_set:
    mov 4(%esp), %eax
    and $0xFFFFF000, %eax # we only care about the highest 20 bits
    or  $0x08, %eax       # we wan't page write through! PWT FTW!
    mov %eax, %cr3        # loads the PDT
    ret

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
