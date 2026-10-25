#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002
#define MULTIBOOT_INFO_MEMORY      0x00000001 
#define MULTIBOOT_INFO_MEM_MAP     0x00000040 
#define MULTIBOOT_MEMORY_AVAILABLE 1

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} __attribute__((packed)) multiboot_info_t;

typedef struct {
    uint32_t size;          // bu alan hariç girdinin boyutu
    uint64_t addr;
    uint64_t len;
    uint32_t type;          // 1 = kullanılabilir RAM
} __attribute__((packed)) multiboot_mmap_entry_t;

#endif