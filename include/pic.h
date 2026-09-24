#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC1            0x20        /* Master PIC I/O temel adresi */
#define PIC2            0xA0        /* Slave PIC I/O temel adresi */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1 + 1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2 + 1)

#define PIC_EOI         0x20        /* End of Interrupt (EOI) komut kodu */

/* ICW1 Sabitleri */
#define ICW1_ICW4       0x01        /* ICW4'ün gönderileceğini belirtir */
#define ICW1_SINGLE     0x02        /* Single mod (0 ise Cascade) */
#define ICW1_INTERVAL4  0x04        /* Call address aralığı 4 (veya 8) */
#define ICW1_LEVEL      0x08        /* Seviye tetiklemeli mod (0 ise Edge) */
#define ICW1_INIT       0x10        /* Başlatma biti - zorunlu! */

/* ICW4 Sabitleri */
#define ICW4_8086       0x01        /* 8086/88 modu */
#define ICW4_AUTO       0x02        /* Otomatik EOI modu */
#define ICW4_BUF_SLAVE  0x08        /* Tamponlu mod / Slave */
#define ICW4_BUF_MASTER 0x0C        /* Tamponlu mod / Master */
#define ICW4_SFNM       0x10        /* Özel tam yuvalanmış (SFNM) mod */

#define CASCADE_IRQ     2           /* Slave PIC'in Master üzerindeki IRQ hattı */

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

static inline void PIC_sendEOI(uint8_t irq)
{
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);

    outb(PIC1_COMMAND, PIC_EOI);
}

static inline void PIC_disable(void)
{
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

/*
 * PIC denetleyicilerini varsayılan 0x08 ve 0x70 adreslerinden 
 * belirtilen kesme vektörlerine taşır (genelde 0x20 ve 0x28).
 */
static inline void PIC_remap(int offset1, int offset2)
{
    // ICW1: Başlatma dizisini başlat (Cascade modu)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // ICW2: Temel vektör ofsetleri
    outb(PIC1_DATA, offset1);                 // Master PIC vektör ofseti (0x20)
    io_wait();
    outb(PIC2_DATA, offset2);                 // Slave PIC vektör ofseti (0x28)
    io_wait();

    // ICW3: Cascade bağlantı yapılandırması
    outb(PIC1_DATA, 1 << CASCADE_IRQ);        // Master'a: IRQ2 pinine Slave bağlı
    io_wait();
    outb(PIC2_DATA, CASCADE_IRQ);             // Slave'e: IRQ2 kimliğine sahipsin
    io_wait();

    // ICW4: x86 / 8086 modu
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Tüm IRQ maskelerini kaldır (kesmeleri dinlemeye aç)
    outb(PIC1_DATA, 0);
    outb(PIC2_DATA, 0);
}

#endif