/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

/* 
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
The multiboot standard does not define the value of the stack pointer register
(esp) and it is up to the kernel to provide a stack. This allocates room for a
small stack by creating a symbol at the bottom of it, then allocating 16384
bytes for it, and finally creating a symbol at the top. The stack grows
downwards on x86. The stack is in its own section so it can be marked nobits,
which means the kernel file is smaller because it does not contain an
uninitialized stack. The stack on x86 must be 16-byte aligned according to the
System V ABI standard and de-facto extensions. The compiler will assume the
stack is properly aligned and failure to align the stack will result in
undefined behavior.
*/
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

/*
The linker script specifies _start as the entry point to the kernel and the
bootloader will jump to this position once the kernel has been loaded. It
doesn't make sense to return from this function as the bootloader is gone.
*/
.section .text
.global _start
.type _start, @function
_start:
	/*
	The bootloader has loaded us into 32-bit protected mode on a x86
	machine. Interrupts are disabled. Paging is disabled. The processor
	state is as defined in the multiboot standard. The kernel has full
	control of the CPU. The kernel can only make use of hardware features
	and any code it provides as part of itself. There's no printf
	function, unless the kernel provides its own <stdio.h> header and a
	printf implementation. There are no security restrictions, no
	safeguards, no debugging mechanisms, only what the kernel provides
	itself. It has absolute and complete power over the
	machine.
	*/

	/*
	To set up a stack, we set the esp register to point to the top of the
	stack (as it grows downwards on x86 systems). This is necessarily done
	in assembly as languages such as C cannot function without a stack.
	*/
	mov $stack_top, %esp

	/*
	This is a good place to initialize crucial processor state before the
	high-level kernel is entered. It's best to minimize the early
	environment where crucial features are offline.
	*/
	call gdt_init



	call idt_init

	/*
	Enter the high-level kernel. The ABI requires the stack is 16-byte
	aligned at the time of the call instruction (which afterwards pushes
	the return pointer of size 4 bytes). The stack was originally 16-byte
	aligned above and we've pushed a multiple of 16 bytes to the
	stack since (pushed 0 bytes so far), so the alignment has thus been
	preserved and the call is well defined.
	*/
	call kernel_main

	/*
	If the system has nothing more to do, put the computer into an
	infinite loop. To do that:
	1) Disable interrupts with cli (clear interrupt enable in eflags).
	   They are already disabled by the bootloader, so this is not needed.
	   Mind that you might later enable interrupts and return from
	   kernel_main (which is sort of nonsensical to do).
	2) Wait for the next interrupt to arrive with hlt (halt instruction).
	   Since they are disabled, this will lock up the computer.
	3) Jump to the hlt instruction if it ever wakes up due to a
	   non-maskable interrupt occurring or due to system management mode.
	*/
	cli
1:	hlt
	jmp 1b

/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start

/*
gdt_flush is called from C (gdt_init) with the pointer to the GDT descriptor
as its first argument (pushed onto the stack at [esp + 4]).

This routine performs three critical tasks:
1) Loads the GDT register (GDTR) using the lgdt instruction.
2) Reloads all data segment registers (DS, ES, FS, GS, SS) with the Kernel Data
   Segment selector (offset 0x10 in our GDT).
3) Performs a far jump (ljmp) to flush the CPU instruction prefetch queue and
   reload the CS (Code Segment) register with the Kernel Code Segment selector
   (offset 0x08 in our GDT).
*/
.global gdt_flush
.type gdt_flush, @function
gdt_flush:
	mov 4(%esp), %eax   /* Fetch the address of the gdt_ptr struct passed as argument */
	lgdt (%eax)         /* Load the GDT into the CPU */

	/* 
	Reload data segment registers with the kernel data selector (0x10).
	0x10 corresponds to the third entry in the GDT (index 2 * 8 bytes = 0x10).
	*/
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

	/* 
	A far jump is mandatory to reload the CS register.
	0x08 corresponds to the second entry in the GDT (index 1 * 8 bytes = 0x08),
	which represents the kernel code segment.
	*/
	ljmp $0x08, $.flush_done

.flush_done:
	ret

.size gdt_flush, . - gdt_flush

.global idt_flush
.type idt_flush, @function
idt_flush:
    mov 4(%esp), %eax   # idt_ptr adresi
    lidt (%eax)         # IDT'yi işlemciye yükle
    ret
.size idt_flush, . - idt_flush

.extern isr_handler

.type isr_common_stub, @function
isr_common_stub:
    pusha                   # EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI sırayla stack'e itilir

    mov %ds, %ax            # Veri segmentini kaydet
    push %eax

    mov $0x10, %ax          # Çekirdek veri segmentine geç (0x10)
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp               # C fonksiyonuna registers struct işaretçisi olarak ESP'yi ilet
    call isr_handler
    add $4, %esp            # Stack'ten argümanı temizle

    pop %eax                # Orijinal segmenti geri yükle
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa                    # Genel amaçlı kayıtçıları geri yükle
    add $8, %esp            # int_no ve err_code alanlarını stack'ten temizle
    iret                    # Kesmeden dön (CS, EIP, EFLAGS CPU tarafından geri yüklenir)

# Makrolar: Hata kodu üretmeyen kesmeler için sahte 0 itilir
.macro ISR_NOERRCODE num
.global isr\num
.type isr\num, @function
isr\num:
    push $0
    push $\num
    jmp isr_common_stub
.endm

# CPU tarafından donanımsal hata kodu üretilen kesmeler
.macro ISR_ERRCODE num
.global isr\num
.type isr\num, @function
isr\num:
    push $\num
    jmp isr_common_stub
.endm

# 0 - 31 Arası İşlemci İstisnaları
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE   30
ISR_NOERRCODE 31