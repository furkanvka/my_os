#include <stdint.h>
#include "isr.h"
#include "timer.h"
#include "keyboard.h"
#include "vga.h"

// Basit string karşılaştırma fonksiyonu (strcmp)
static int k_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

// Terminal Shell Döngüsü
void run_shell(void) {
    char cmd_buffer[80];
    int cmd_len = 0;

    terminal_clear();
    terminal_writestring("Welcome to my os \nmy_os> ");

    while (1) {
        char c = kbd_getchar();

        if (c == '\n') {
            cmd_buffer[cmd_len] = '\0';
            terminal_putchar('\n');

            if (cmd_len > 0) {
                if (k_strcmp(cmd_buffer, "clear") == 0) {
                    terminal_clear();
                } else if (k_strcmp(cmd_buffer, "help") == 0) {
                    terminal_writestring("Mevcut komutlar: help, clear, ping\n");
                } else if (k_strcmp(cmd_buffer, "ping") == 0) {
                    terminal_writestring("pong!\n");
                } else {
                    terminal_writestring("Bilinmeyen komut: ");
                    terminal_writestring(cmd_buffer);
                    terminal_putchar('\n');
                }
            }

            cmd_len = 0;
            terminal_writestring("my_os> ");
        } 
        else if (c == '\b') {
            if (cmd_len > 0) {
                cmd_len--;
                terminal_putchar('\b');
            }
        } 
        else {
            cmd_buffer[cmd_len++] = c;
            terminal_putchar(c);
        }
    }
}

void kernel_main(void) 
{
    timer_init(100);
    keyboard_init();

    __asm__ __volatile__("sti");

    run_shell();

    // Güvenlik amaçlı bekleme döngüsü
    while(1) {
        __asm__ __volatile__("hlt");
    }
}