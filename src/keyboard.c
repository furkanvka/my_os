#include <stdint.h>
#include "pic.h"
#include "isr.h"

#define KBD_BUFFER_SIZE 256

// Scancode Set 1 tablosu (Make kodları: 0x00 - 0x3A)
static const char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',   0, ' '
};

// Ring buffer veri yapısı
static char kbd_buffer[KBD_BUFFER_SIZE];
static volatile uint16_t kbd_head = 0; // Kesmenin yazdığı yer
static volatile uint16_t kbd_tail = 0; // Terminalin okuduğu yer

static void kbd_buffer_push(char c) {
    uint16_t next = (kbd_head + 1) % KBD_BUFFER_SIZE;
    // Tampon dolu değilse veriyi ekle
    if (next != kbd_tail) {
        kbd_buffer[kbd_head] = c;
        kbd_head = next;
    }
}

// Tamponda okunmamış karakter var mı?
int kbd_has_key(void) {
    return kbd_head != kbd_tail;
}

// Terminalin çağırdığı bloke edici okuma fonksiyonu
char kbd_getchar(void) {
    while (!kbd_has_key()) {
        __asm__ volatile("hlt");
    }

    char c = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return c;
}

static void keyboard_callback(struct registers *regs) {
    (void)regs;
    uint8_t scancode = inb(0x60);

    // Sadece tuşa basılma anını yakala, bırakılmaları yoksay
    if (!(scancode & 0x80)) {
        if (scancode < sizeof(kbd_us)) {
            char c = kbd_us[scancode];
            if (c != 0) {
                kbd_buffer_push(c); // Sadece kuyruğa atıp kesmeden çık
            }
        }
    }
}

void keyboard_init(void) {
    register_interrupt_handler(33, keyboard_callback);
}