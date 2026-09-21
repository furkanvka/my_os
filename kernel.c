#include "isr.h"

void kernel_main(void) 
{
    // IDT girdilerine kapıları bağla
    isr_install();

    char *video_memory = (char *) 0xB8000;
    const char *str = "IDT ve ISR Hazir!";
    
    int i = 0;
    while (str[i] != '\0') {
        video_memory[i * 2] = str[i];
        video_memory[i * 2 + 1] = 0x07;
        i++;
    }

    // --- TEST KISMI ---
    // Bilerek bir 0'a bölme istisnası (ISR 0) fırlat:
    __asm__ __volatile__("int $0");

    while(1);
}