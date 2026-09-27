#ifndef VGA_H
#define VGA_H

#include <stdint.h>

void terminal_clear(void);
void terminal_putchar(char c);
void terminal_writestring(const char *str);

#endif