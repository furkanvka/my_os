#include "isr.h"
#include "keyboard.h"
#include "timer.h"
#include "vga.h"
#include "pmm.h"
#include "multiboot.h"
#include <stdint.h>

// Linker script'te tanımlanan semboller
extern uint8_t kernel_start[], endkernel[];
extern uint8_t text_start[],   text_end[];
extern uint8_t rodata_start[], rodata_end[];
extern uint8_t data_start[],   data_end[];
extern uint8_t bss_start[],    bss_end[];

// boot.s'te tanımlanan stack sınırları (.global olmalı)
extern uint8_t stack_bottom[], stack_top[];

// Basit string karşılaştırma fonksiyonu (strcmp)
static int k_strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    return *(const unsigned char *)s1 -
           *(const unsigned char *)s2;
}

// Unsigned integer değerini terminale yazdırır.
static void terminal_write_uint(uint32_t value)
{
    char buffer[11];
    int i = 0;

    if (value == 0) {
        terminal_putchar('0');
        return;
    }

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        terminal_putchar(buffer[--i]);
    }
}

// Değeri 0xXXXXXXXX biçiminde yazdırır.
static void terminal_write_hex(uint32_t value)
{
    const char *hex = "0123456789ABCDEF";

    terminal_writestring("0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        terminal_putchar(hex[(value >> shift) & 0xF]);
    }
}

// 1 KiB'tan küçükse byte, değilse KiB olarak yazar.
static void terminal_write_size(uint32_t bytes)
{
    if (bytes < 1024) {
        terminal_write_uint(bytes);
        terminal_writestring(" B");
    } else {
        terminal_write_uint(bytes / 1024);
        terminal_writestring(" KiB");
    }
}

// Frame sayısını boyut + frame adedi olarak yazar.
static void print_frames(uint32_t frames)
{
    terminal_write_size(frames * PAGE_SIZE);
    terminal_writestring(" (");
    terminal_write_uint(frames);
    terminal_writestring(" frame)\n");
}

// Bir kernel bölümünün boyutunu yazar.
static void print_section(const char *name, uint8_t *start, uint8_t *end)
{
    terminal_writestring(name);
    terminal_write_size((uint32_t)(end - start));
    terminal_putchar('\n');
}

// Bellek bilgilerini terminale yazdırır.
static void print_memory_info(void)
{
    uint32_t k_start = (uint32_t)(uintptr_t)kernel_start;
    uint32_t k_end   = (uint32_t)(uintptr_t)endkernel;

    uint32_t total_frames = pmm_get_total_frames();
    uint32_t free_frames  = pmm_get_free_frames();
    uint32_t used_frames  = total_frames - free_frames;

    // 0'dan kernel sonuna kadar olan frame'ler (BIOS/VGA + kernel)
    uint32_t reserved_frames = (k_end + PAGE_SIZE - 1) / PAGE_SIZE;

    // kalloc_frame ile alınanlar
    uint32_t allocated_frames = 0;
    if (used_frames > reserved_frames) {
        allocated_frames = used_frames - reserved_frames;
    }

    terminal_writestring("== Fiziksel Bellek ==\n");

    terminal_writestring("Toplam      : ");
    terminal_write_uint(pmm_get_total_memory() / (1024 * 1024));
    terminal_writestring(" MiB (");
    terminal_write_uint(total_frames);
    terminal_writestring(" frame)\n");

    terminal_writestring("Bos         : ");
    print_frames(free_frames);

    terminal_writestring("Kullanilan  : ");
    print_frames(used_frames);

    terminal_writestring("  Rezerve   : ");
    print_frames(reserved_frames);

    terminal_writestring("  kalloc    : ");
    print_frames(allocated_frames);

    terminal_writestring("Doluluk     : %");
    if (total_frames > 0) {
        terminal_write_uint((used_frames * 100) / total_frames);
    } else {
        terminal_putchar('0');
    }
    terminal_putchar('\n');

    terminal_writestring("\n== Kernel ==\n");

    terminal_writestring("Adres       : ");
    terminal_write_hex(k_start);
    terminal_writestring(" - ");
    terminal_write_hex(k_end);
    terminal_putchar('\n');

    terminal_writestring("Boyut       : ");
    terminal_write_size(k_end - k_start);
    terminal_putchar('\n');

    print_section("  .text     : ", text_start,   text_end);
    print_section("  .rodata   : ", rodata_start, rodata_end);
    print_section("  .data     : ", data_start,   data_end);

    terminal_writestring("  .bss      : ");
    terminal_write_size((uint32_t)(bss_end - bss_start));
    terminal_writestring(" (frame_map: ");
    terminal_write_size(MAX_FRAMES);
    terminal_writestring(")\n");

    print_section("    stack   : ", stack_bottom, stack_top);

    terminal_writestring("Dusuk bellek: ");
    terminal_write_size(k_start);
    terminal_writestring(" (BIOS/VGA)\n");
}

// Terminal Shell Döngüsü
void run_shell(void)
{
    char cmd_buffer[80];
    int cmd_len = 0;

    terminal_clear();
    terminal_writestring("Welcome to my os\nmy_os> ");

    while (1) {
        char c = kbd_getchar();

        if (c == '\n') {
            cmd_buffer[cmd_len] = '\0';
            terminal_putchar('\n');

            if (cmd_len > 0) {

                if (k_strcmp(cmd_buffer, "clear") == 0) {
                    terminal_clear();

                } else if (k_strcmp(cmd_buffer, "help") == 0) {
                    terminal_writestring(
                        "Mevcut komutlar: help, clear, ping, color, mem\n"
                    );

                } else if (k_strcmp(cmd_buffer, "ping") == 0) {
                    terminal_writestring("pong!\n");

                } else if (k_strcmp(cmd_buffer, "mem") == 0) {
                    print_memory_info();

                } else if (k_strcmp(cmd_buffer, "color") == 0) {
                    terminal_writestring(
                        "Kullanim: color "
                        "red|green|blue|yellow|cyan|magenta|white|gray\n"
                    );

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

void kernel_main(uint32_t magic, multiboot_info_t *mbi)
{
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        terminal_writestring("Hata: multiboot ile boot edilmedi!\n");
        while (1) {
            __asm__ __volatile__("cli; hlt");
        }
    }

    pmm_init(mbi);

    timer_init(100);
    keyboard_init();

    __asm__ __volatile__("sti");

    run_shell();

    // Güvenlik amaçlı bekleme döngüsü
    while (1) {
        __asm__ __volatile__("hlt");
    }
}