#ifndef ISR_H
#define ISR_H

#include <stdint.h>

struct registers {
    uint32_t ds;                                     // push %eax ile itilen veri segmenti
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // pusha ile itilenler
    uint32_t int_no, err_code;                       // Assembly stub tarafından itilenler
    uint32_t eip, cs, eflags, useresp, ss;           // İşlemci tarafından donanımsal itilenler
} __attribute__((packed));

typedef void (*isr_t)(struct registers *);

void isr_install(void);
void isr_handler(struct registers *r);
void register_interrupt_handler(uint8_t n, isr_t handler);

#endif