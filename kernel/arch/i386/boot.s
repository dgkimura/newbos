/* Declare constraints for the muliboot header. */
.set ALIGN,     1<<0            /* align loaded modules on page boundaries */
.set MEMINFO,   1<<1            /* provide memory map */
.set FLAGS,     ALIGN | MEMINFO /* this is the Multiboot 'flag' field */
.set MAGIC,     0x1BADB002      /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

/*
 * Declare a multiboot header that marks the program as a kernel. These are
 * magic values that are documented in the multiboot standard. The bootloader
 * will search for this signature in the first 8 KiB of the kernel file,
 * aligned at a 32-bit boundary. The signature is in its own section so the
 * header can be forced to be within the first 8 KiB of the kernel file.
 */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
 * The multiboot standard does not define the value of the stack pointer
 * register (esp) and it is up to the kernel to provide a stack. This allocates
 * room for a small stack by creating a symbol at the bottom of it, then
 * allocating 16384 bytes for it, and finally creating a symbol at the top. The
 * stack grows downwards on x86. The stack is in its own section so it can be
 * marked nobits, which means the kernel file is smaller because it does not
 * contain an uninitialized stack. The stack on x86 must be 16-byte aligned
 * according to the System V ABI standard and de-facto extensions. The compiler
 * will assume the stack is properly aligned and failure to align the stack
 * will result in undefined behavior.
 */
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.set KERNEL_START_VADDR,    0xC0000000
.set KERNEL_PDT_IDX,       (KERNEL_START_VADDR >> 22)

/*
 * Declares the kernel data structures: page directory and page table.
 */
.section .data
.align 4096
kernel_pt:
.fill 1024, 4, 0

kernel_pdt:
.long 0x0000008B    /* flags: global, present, readwrite, writethrough */
.fill 1023, 4, 0

/*
 * Grub boot info at a known accessible location after we enable high half
 * kernel paging.
 */
.section .data
.align 4
multiboot_info:
.skip 4

grub_magic_number:
.skip 4

/*
 * The linker script specifies _start as the entry point to the kernel and the
 * bootloader will jump to this position once the kernel has been loaded. It
 * doesn't make sense to return from this function as the bootloader is gone.
 */
.section .text
.align 4

.global _start
.type _start, @function
_start:
    /*
     * The bootloader has loaded us into 32-bit protected mode on a x86
     * machine. Interrupts are disabled. Paging is disabled. The processor
     * state is as defined in the multiboot standard. The kernel has full
     * control of the CPU. The kernel can only make use of hardware features
     * and any code it provides as part of itself. There's no printf function,
     * unless the kernel provides its own <stdio.h> header and a printf
     * implementation. There are no security restrictions, no safeguards, no
     * debugging mechanisms, only what the kernel provides itself. It has
     * absolute and complete power over the machnine.
     */

    /*
     * To set up a stack, we set the esp register to point to the top of our
     * stack (as it grows downwards on x86 systems). This is necessarily done
     * in assembly as languages such as C cannot function without a stack.
     */
    mov $stack_top, %esp

    /*
     * To copy grub boot info to a known location accessible after we enable
     * high half kernel paging.
     */
    mov $(grub_magic_number - KERNEL_START_VADDR), %ecx
    mov %eax, (%ecx)
    mov $(multiboot_info - KERNEL_START_VADDR), %ecx
    mov %ebx, (%ecx)

    /*
     * This is a good place to initialize crucial processor state before the
     * high-level kernel is entered. It's best to minimize the early
     * environment where crucial features are offline. Note that the processor
     * is not fully initialized yet: Features such as floating point
     * instructions and instruction set extentions are not initialized yet. The
     * GDT should be loaded here. Paging should be enabled here.  C++ features
     * such as global cunstructors and exceptions will require runtime support
     * to work here as well.
     */
setup_kernel_pdt:
    mov $(kernel_pdt - KERNEL_START_VADDR + KERNEL_PDT_IDX*4), %ecx
    mov $(kernel_pt - KERNEL_START_VADDR), %edx
    or  $0x0000000B, %edx # write-through, writable, present
    mov %edx, (%ecx)

setup_kernel_pt:
    mov $(kernel_pt - KERNEL_START_VADDR), %eax
    mov $0x0000000B, %ecx # write-through, writable, present

loop:
    mov %ecx, (%eax)
    add $4, %eax
    add $0x1000, %ecx
    cmp $kernel_physical_end, %ecx
    jle loop

enable_paging:
    mov $(kernel_pdt - KERNEL_START_VADDR), %ecx

    and $0xFFFFF000, %ecx # we only care about the upper 20 bits
    or  $0x08,%ecx        # PWT, enable page write through?
    mov %ecx, %cr3        # load pdt

    mov %cr4, %ecx        # read current config from cr4
    or  $0x00000010, %ecx # set bit enabling 4MB pages
    mov %ecx, %cr4        # enable it by writing to cr4

    mov %cr0, %ecx        # read current config from cr0
    or  $0x80000000, %ecx # the highest bit controls paging
    mov %ecx, %cr0        # enable paging by writing config to cr0

    /*
     * Grub bootloader specification states that EBX must contain the 32-bit
     * physical address of the multiboot information structure.
     */
    push grub_magic_number - KERNEL_START_VADDR
    push multiboot_info - KERNEL_START_VADDR
    push $kernel_pt
    push $kernel_pdt
    push $kernel_virtual_end
    push $kernel_virtual_start
    push $kernel_physical_end
    push $kernel_physical_start

    /*
     * Enter the high-level kernel. The ABI requires the stack is 16-byte
     * aligned at the time of the call instruction (which afterwards pushes the
     * return pointer of size 4 bytes). The stack was originally 16-byte
     * aligned above and we've since pushed a multiple of 16-bytes to the stack
     * since (pushed 0 bytes so far) and the alignment is thus preserved and
     * the call is well defined.
     */
    call kernel_main

    /*
     * If the system has nothing more to do, put the computer into an infinite
     * loop. To do that:
     * 1) Disable interrupts with cli (clear interrupt enable in eflags).  They
     *    are already disabled by the bootloader, so this is not needed.  Mind
     *    that you might later enable interrupts and return from kernel_main
     *    (which is sort of nonsensical to do).
     * 2) Wait for the next interrupt to arrive with hlt (halt instruction).
     *    Since they are disabled, this will lock up the computer.
     * 3) Jump to the hlt instruction if it ever wakes up due to a non-maskable
     *    interrupt occurring or due to system management mode.
     */
    cli

hang:
    jmp hang
