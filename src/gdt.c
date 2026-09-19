#include "gdt.h"

struct gdt_entry gdt_entries[3];
struct gdt_ptr   gdt_pointer;

// gdt_flush fonksiyonu boot.s içinde tanımlı olacak
extern void gdt_flush(uint32_t);

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low     = (base & 0xFFFF);
    gdt_entries[num].base_middle  = (base >> 16) & 0xFF;
    gdt_entries[num].base_high    = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low    = (limit & 0xFFFF);
    gdt_entries[num].granularity  = ((limit >> 16) & 0x0F);

    gdt_entries[num].granularity |= (gran & 0xF0);
    gdt_entries[num].access       = access;
}

void gdt_init(void) {
    gdt_pointer.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gdt_pointer.base  = (uint32_t)&gdt_entries;

    // 0: Null Segment
    gdt_set_gate(0, 0, 0, 0, 0);

    // 1: Kernel Code Segment (Offset: 0x08)
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);

    // 2: Kernel Data Segment (Offset: 0x10)
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);

    // Assembly tarafındaki flush rutinini çağırıyoruz
    gdt_flush((uint32_t)&gdt_pointer);
}