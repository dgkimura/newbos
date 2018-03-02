#include <newbos/kmalloc.h>
#include <newbos/paging.h>
#include <newbos/printk.h>
#include <newbos/timer.h>
#include <newbos/tty.h>

#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"
#include "multiboot.h"

void
kernel_main(
    uint32_t kernel_physical_start,
    uint32_t kernel_physical_end,
    uint32_t kernel_virtual_start,
    uint32_t kernel_virtual_end,
    uint32_t kernel_pdt_vaddr,
    uint32_t kernel_pt_vaddr,
    struct multiboot_info *multiboot_info)
{
    gdt_init();
    clear_idt();

    interrupts_init();

    enable_interrupts();

    tty_clear();
    printk("Welcome to newbos...\n");

    frames_init();

    kmalloc_init((void *)KMALLOC_START, KMALLOC_LENGTH);

    //asm volatile ("int $0x3");
    //asm volatile ("int $0x4");

    keyboard_init();

    //timer_init(1000);

    //process_init();

    // Loop forever.
    for (;;);
}
