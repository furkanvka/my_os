#include "timer.h"
#include "isr.h"
#include "pic.h"

static uint32_t timer_ticks = 0;

static void timer_callback(struct registers *regs) {
    (void)regs;
    timer_ticks++;

    // Doğrulama: Her 100 tick'te (1 saniyede bir)
    if (timer_ticks % 100 == 0) {
        char *video = (char *)0xB8000;
        video[158] = (video[158] == '|') ? '/' : '|'; // Yanıp sönen imleç efekti
        video[159] = 0x0F;
    }
}

void timer_init(uint32_t frequency) {
    register_interrupt_handler(32, timer_callback);

    uint32_t divisor = 1193180 / frequency;

    // 0x36: Kanal 0, lo/hi byte erişimi, Kare Dalga Modu (Mode 3)
    outb(0x43, 0x36);

    // Bölen değerini önce alt (low), sonra üst (high) bayt olarak porta yaz
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}