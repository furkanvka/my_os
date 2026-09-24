#include "keyboard.h"
#include "isr.h"
#include "pic.h"

static const char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, /* Ctrl */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, /* Left Shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, /* Right Shift */
    '*',
    0,  /* Alt */
    ' ', /* Space */
};

static void keyboard_callback(struct registers *regs) {
    (void)regs;
    uint8_t scancode = inb(0x60);

    if (!(scancode & 0x80)) {
        char c = kbd_us[scancode];
        if (c != 0) {
            // Ekrana doğrudan bas (Örnek: 10. satır)
            static int cursor_col = 0;
            char *video = (char *)0xB8000;
            int offset = (10 * 80 + cursor_col) * 2;
            
            video[offset] = c;
            video[offset + 1] = 0x0F;
            cursor_col++;
        }
    }
}

void keyboard_init(void) {
    // IRQ1 (Vektör 33) işleyicisini kaydet
    register_interrupt_handler(33, keyboard_callback);
}