#include "isr.h"
#include "timer.h"
#include "keyboard.h"

void kernel_main(void) 
{
    timer_init(100);     // 100 Hz zamanlayıcı
    keyboard_init();     // Klavye dinleyicisi

    // 2. CPU kesmelerini aç
    __asm__ __volatile__("sti");

    char *video_memory = (char *) 0xB8000;
    const char *str = "Zamanlayici ve Klavye Calisiyor. Tusa basin:";
    for (int i = 0; str[i] != '\0'; i++) {
        video_memory[i * 2] = str[i];
        video_memory[i * 2 + 1] = 0x0A; // Yeşil yazı
    }

    // Çekirdeğin sonlanmasını önle
    while(1) {
        __asm__ __volatile__("hlt"); // Kesme gelene kadar CPU'yu uyut (güç tasarrufu)
    }
}