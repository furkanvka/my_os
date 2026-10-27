#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include "multiboot.h"

#define PAGE_SIZE    4096u
#define MAX_RAM_SIZE (512u * 1024u * 1024u)
#define MAX_FRAMES   (MAX_RAM_SIZE / PAGE_SIZE)

typedef uint32_t pageframe_t;

void pmm_init(multiboot_info_t *mbi);
pageframe_t kalloc_frame(void);
void kfree_frame(uint32_t addr);

uint32_t pmm_get_total_frames(void);
uint32_t pmm_get_total_memory(void);
uint32_t pmm_get_free_frames(void);
uint32_t pmm_get_free_memory(void);

#endif