.global outb
.type outb, @function
outb:
    mov 0x4(%esp), %dx
	mov 0x8(%esp), %al 
	out %al, (%dx)
    ret

.global inb
.type inb, @function
inb:
	mov 0x4(%esp), %dx
	in  (%dx), %al
    ret

.global inw
.type inw, @function
inw:
	mov 0x4(%esp), %dx
    in  (%dx),%ax
    ret
