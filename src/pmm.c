#include "pmm.h"

extern uint8_t endkernel[];

static uint8_t  frame_map[MAX_FRAMES / 8];
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
        uint32_t frame = addr / PAGE_SIZE;

        if (frame >= first_free_frame) {
            frame_map[frame / 8] &= ~(1U << (frame % 8));
        }
    }
}

void pmm_init(multiboot_info_t *mbi)
{
    for (uint32_t i = 0; i < MAX_FRAMES; i++) {
        frame_map[i / 8] |= (1U << (i % 8));
    }

    total_frames     = 0;
    first_free_frame = ALIGN_UP((uint32_t)(uintptr_t)endkernel) / PAGE_SIZE;

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
}

pageframe_t kalloc_frame(void)
{
    for (uint32_t i = first_free_frame; i < total_frames; i++) {
        uint32_t byte_index = i / 8;
        uint32_t bit_index  = i % 8;

        if ((frame_map[byte_index] & (1U << bit_index)) == 0) {
            frame_map[byte_index] |= (1U << bit_index);
            return (pageframe_t)(i * PAGE_SIZE);
        }
    }

    return 0;
}

void kfree_frame(uint32_t addr)
{
    if (addr % PAGE_SIZE != 0)
        return;

    uint32_t frame = addr / PAGE_SIZE;

    if (frame < first_free_frame || frame >= total_frames)
        return;

    uint32_t byte_index = frame / 8;
    uint32_t bit_index  = frame % 8;

    frame_map[byte_index] &= ~(1U << bit_index);
}

uint32_t pmm_get_total_frames(void) { return total_frames; }
uint32_t pmm_get_total_memory(void) { return total_frames * PAGE_SIZE; }

uint32_t pmm_get_free_frames(void)
{
    uint32_t n = 0;

    for (uint32_t i = first_free_frame; i < total_frames; i++) {
        uint32_t byte_index = i / 8;
        uint32_t bit_index  = i % 8;

        if ((frame_map[byte_index] & (1U << bit_index)) == 0)
            n++;
    }

    return n;
}

uint32_t pmm_get_free_memory(void)
{
    return pmm_get_free_frames() * PAGE_SIZE;
}