#include <newbos/kmalloc.h>
#include <newbos/paging.h>
#include <newbos/process.h>
#include <newbos/printk.h>
#include <newbos/scheduler.h>
#include <newbos/timer.h>
#include <newbos/tty.h>

#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"
#include "memory.h"
#include "multiboot.h"

void
kernel_main(
    uint32_t kernel_physical_start,
    uint32_t kernel_physical_end,
    uint32_t kernel_virtual_start,
    uint32_t kernel_virtual_end,
    uint32_t kernel_pdt_vaddr,
    uint32_t kernel_pt_vaddr,
    struct multiboot_info *minfo,
    uint32_t magic_number)
{
    uint32_t tss_vaddr = tss_init();
    gdt_init(tss_vaddr);
    clear_idt();

    interrupts_init();

    enable_interrupts();

    tty_clear();

    if (magic_number != MULTIBOOT_BOOTLOADER_MAGIC) {
        printk("ERROR: magic number '%X' is wrong!\n", magic_number);
        return;
    }

    printk("Welcome to newbos...\n");

    frames_init(kernel_physical_start, kernel_physical_end,
                kernel_virtual_start, kernel_virtual_end,
                VIRTUAL_TO_PHYSICAL(kernel_pdt_vaddr),
                VIRTUAL_TO_PHYSICAL(kernel_pt_vaddr),
                minfo);

    //asm volatile ("int $0x3");
    //asm volatile ("int $0x4");

    keyboard_init();

    int *i = (int *)kmalloc(sizeof(int));
    *i = 42;
    printk("kmalloc'd '%u' at %X...\n", *i, i);

    //timer_init(1000);

    struct process *p = process_create("/bin/init");
    scheduler_add_process(p);
    scheduler_schedule();
    printk("Finished process init %u!!!\n", p->id);

    // Loop forever.
    for (;;);
}
