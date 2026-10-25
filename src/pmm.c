#include "pmm.h"

extern uint8_t endkernel[];

static uint8_t  frame_map[MAX_FRAMES];
static uint32_t total_frames;
static uint32_t first_free_frame;

#define ALIGN_UP(x)   (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define ALIGN_DOWN(x) ((x) & ~(PAGE_SIZE - 1))

static void mark_region_free(uint32_t start, uint32_t end)
{
    start = ALIGN_UP(start);
    end   = ALIGN_DOWN(end);

    if (end / PAGE_SIZE > total_frames)
        total_frames = end / PAGE_SIZE;

    for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
        if (addr / PAGE_SIZE >= first_free_frame)
            frame_map[addr / PAGE_SIZE] = FREE;
    }
}

void pmm_init(multiboot_info_t *mbi)
{
    for (uint32_t i = 0; i < MAX_FRAMES; i++)
        frame_map[i] = USED;

    total_frames     = 0;
    first_free_frame = ALIGN_UP((uint32_t)(uintptr_t)endkernel) / PAGE_SIZE;

    if (mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
        // En doğru yöntem: BIOS'un bellek haritası
        uint32_t ptr = mbi->mmap_addr;
        uint32_t end = mbi->mmap_addr + mbi->mmap_length;

        while (ptr < end) {
            multiboot_mmap_entry_t *e = (multiboot_mmap_entry_t *)ptr;

            if (e->type == MULTIBOOT_MEMORY_AVAILABLE && e->addr < MAX_RAM_SIZE) {
                uint64_t r_end = e->addr + e->len;
                if (r_end > MAX_RAM_SIZE)
                    r_end = MAX_RAM_SIZE;

                mark_region_free((uint32_t)e->addr, (uint32_t)r_end);
            }

            ptr += e->size + sizeof(e->size);
        }

    } else if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
        // Yedek yöntem: 1 MiB üstündeki kesintisiz bellek miktarı
        uint64_t r_end = 0x100000ull + (uint64_t)mbi->mem_upper * 1024;
        if (r_end > MAX_RAM_SIZE)
            r_end = MAX_RAM_SIZE;

        mark_region_free(0x100000, (uint32_t)r_end);
    }
}

pageframe_t kalloc_frame(void)
{
    for (uint32_t i = first_free_frame; i < total_frames; i++) {
        if (frame_map[i] == FREE) {
            frame_map[i] = USED;
            return i * PAGE_SIZE;
        }
    }
    return 0;
}

void kfree_frame(uint32_t addr)
{
    uint32_t frame = addr / PAGE_SIZE;

    if (addr % PAGE_SIZE != 0)
        return;
    if (frame < first_free_frame || frame >= total_frames)
        return;

    frame_map[frame] = FREE;
}

uint32_t pmm_get_total_frames(void) { return total_frames; }
uint32_t pmm_get_total_memory(void) { return total_frames * PAGE_SIZE; }

uint32_t pmm_get_free_frames(void)
{
    uint32_t n = 0;
    for (uint32_t i = 0; i < total_frames; i++)
        if (frame_map[i] == FREE)
            n++;
    return n;
}

uint32_t pmm_get_free_memory(void)
{
    return pmm_get_free_frames() * PAGE_SIZE;
}