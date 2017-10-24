#include <stdint.h>

/*
 * The symbol table for a.out.
 */
typedef struct multiboot_aout_symbol_table
{
  uint32_t tabsize;
  uint32_t strsize;
  uint32_t addr;
  uint32_t reserved;
} multiboot_aout_symbol_table_t;

/*
 * The section header table for ELF.
 */
typedef struct multiboot_elf_section_header_table
{
  uint32_t num;
  uint32_t size;
  uint32_t addr;
  uint32_t shndx;
} multiboot_elf_section_header_table_t;

/*
 * Multiboot information data structure defined by GNU bootloader specification
 */
typedef struct multiboot
{
    /*
     * Multiboot info version number
     */
    uint32_t flags;

    /*
     * Available memeory from BIOS
     */
    uint32_t mem_lower;
    uint32_t mem_upper;

    /*
     * "root partition
     */
    uint32_t boot_device;

    /*
     * Kernel command line
     */
    uint32_t cmdline;

    /*
     * Boot-Module list
     */
    uint32_t mods_count;
    uint32_t mods_addr;

    union
    {
        multiboot_aout_symbol_table_t aout_sym;
        multiboot_elf_section_header_table_t elf_sec;
    } u;

    /*
     * Memory Mapping buffer
     */
    uint32_t mmap_length;
    uint32_t mmap_addr;

    /*
     * Drive Info buffer
     */
    uint32_t drives_length;
    uint32_t drives_addr;

    /*
     * ROM configuration table
     */
    uint32_t config_table;

    /*
     * Boot Loader Name
     */
    uint32_t boot_loader_name;

    /*
     * APM table
     */
    uint32_t apm_table;
} multiboot_t;
