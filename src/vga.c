#include <stdint.h>
#include "pic.h"

static volatile uint16_t *const VGA_MEMORY = (volatile uint16_t *)0xB8000;


static int term_row = 0;
static int term_col = 0;
static uint8_t term_color = 0x07; // Siyah arka plan, açık gri yazı

static void update_hardware_cursor(void) {
    uint16_t pos = term_row * 80 + term_col;
    outb(0x3D4, 14);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    outb(0x3D4, 15);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}

void terminal_clear(void) {
    uint16_t blank = (term_color << 8) | ' ';
    for (int i = 0; i < 80 * 25; i++) {
        VGA_MEMORY[i] = blank;
    }
    term_row = 0;
    term_col = 0;
    update_hardware_cursor();
}

void terminal_putchar(char c) {
    if (c == '\n') {
        term_col = 0;
        term_row++;
    } else if (c == '\b') {
        if (term_col > 0) {
            term_col--;
            int index = term_row * 80 + term_col;
            VGA_MEMORY[index] = (term_color << 8) | ' ';
        }
    } else {
        int index = term_row * 80 + term_col;
        VGA_MEMORY[index] = (term_color << 8) | c;
        term_col++;
        if (term_col >= 80) {
            term_col = 0;
            term_row++;
        }
    }

    // Ekranın altına gelindiyse basit başa sarma
    if (term_row >= 25) {
        term_row = 0;
    }

    update_hardware_cursor();
}

void terminal_writestring(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
}