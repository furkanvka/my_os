CC = i686-elf-gcc
AS = i686-elf-as

CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Iinclude
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib

BINDIR = bin
SRCDIR = src

KERNEL_C = kernel.c
BOOT_S = boot.s
SRC_C = $(wildcard $(SRCDIR)/*.c)

KERNEL_OBJS = $(BINDIR)/boot.o $(BINDIR)/kernel.o
SRC_OBJS = $(patsubst $(SRCDIR)/%.c, $(BINDIR)/%.o, $(SRC_C))
ALL_OBJS = $(KERNEL_OBJS) $(SRC_OBJS)

KERNEL_BIN = $(BINDIR)/myos
ISO_BIN = $(BINDIR)/myos.iso

all: $(ISO_BIN)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/boot.o: $(BOOT_S) | $(BINDIR)
	$(AS) $< -o $@

$(BINDIR)/kernel.o: $(KERNEL_C) | $(BINDIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BINDIR)/%.o: $(SRCDIR)/%.c | $(BINDIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(KERNEL_BIN): $(ALL_OBJS) linker.ld
	$(CC) $(LDFLAGS) -o $@ $(ALL_OBJS) -lgcc

$(ISO_BIN): $(KERNEL_BIN) grub.cfg
	mkdir -p $(BINDIR)/isodir/boot/grub
	cp $(KERNEL_BIN) $(BINDIR)/isodir/boot/myos
	cp grub.cfg $(BINDIR)/isodir/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(BINDIR)/isodir
	rm -rf $(BINDIR)/isodir

run: $(ISO_BIN)
	qemu-system-i386 -cdrom $(ISO_BIN) -display sdl

clean:
	rm -rf $(BINDIR) *.o myos myos.iso

.PHONY: all run clean