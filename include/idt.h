#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// 32-bit IDT Kapı Tanımlayıcısı (8 Bayt)
struct idt_entry {
    uint16_t offset_low;  // İşleyici fonksiyon adresinin alt 16 biti (0..15)
    uint16_t selector;    // GDT Kernel Code Segment seçicisi (0x08)
    uint8_t  zero;        // Kullanılmaz, daima 0
    uint8_t  type_attr;   // P, DPL, Gate Type (Genellikle 0x8E)
    uint16_t offset_high; // İşleyici fonksiyon adresinin üst 16 biti (16..31)
} __attribute__((packed));

// lidt komutuna verilecek işaretçi
struct idt_ptr {
    uint16_t limit;       // IDT tablosunun boyutu - 1
    uint32_t base;        // IDT tablosunun başlangıç adresi
} __attribute__((packed));

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif