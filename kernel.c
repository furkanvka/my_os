#include "isr.h"
#include "keyboard.h"
#include "timer.h"
#include "vga.h"
#include <stdint.h>

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
          terminal_writestring("Mevcut komutlar: help, clear, ping, color\n");
        } else if (k_strcmp(cmd_buffer, "ping") == 0) {
          terminal_writestring("pong!\n");
        } else if (k_strcmp(cmd_buffer, "color") == 0) {
          terminal_writestring(
              "Kullanim: color "
              "red|green|blue|yellow|cyan|magenta|white|gray\n");
        } else if (k_strcmp(cmd_buffer, "color red") == 0) {
          update_color(4, 0);
        } else if (k_strcmp(cmd_buffer, "color green") == 0) {
          update_color(2, 0);
        } else if (k_strcmp(cmd_buffer, "color blue") == 0) {
          update_color(1, 0);
        } else if (k_strcmp(cmd_buffer, "color yellow") == 0) {
          update_color(14, 0);
        } else if (k_strcmp(cmd_buffer, "color cyan") == 0) {
          update_color(11, 0);
        } else if (k_strcmp(cmd_buffer, "color magenta") == 0) {
          update_color(13, 0);
        } else if (k_strcmp(cmd_buffer, "color white") == 0) {
          update_color(15, 0);
        } else if (k_strcmp(cmd_buffer, "color gray") == 0) {
          update_color(7, 0);
        } else {
          terminal_writestring("Bilinmeyen komut: ");
          terminal_writestring(cmd_buffer);
          terminal_putchar('\n');
        }
      }

      cmd_len = 0;
      terminal_writestring("my_os> ");
    } else if (c == '\b') {
      if (cmd_len > 0) {
        cmd_len--;
        terminal_putchar('\b');
      }
    } else {
      if (cmd_len < 79) {
        cmd_buffer[cmd_len++] = c;
        terminal_putchar(c);
      }
    }
  }
}

void kernel_main(void) {
  timer_init(100);
  keyboard_init();

  __asm__ __volatile__("sti");

  run_shell();

  // Güvenlik amaçlı bekleme döngüsü
  while (1) {
    __asm__ __volatile__("hlt");
  }
}