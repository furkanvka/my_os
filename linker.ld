ENTRY(_start)

SECTIONS
{
    . = 1M;
    kernel_start = .;

    .text :
    {
        text_start = .;
        *(.multiboot)
        *(.text)
        text_end = .;
    }

    .rodata : ALIGN(4K)
    {
        rodata_start = .;
        *(.rodata*)
        rodata_end = .;
    }

    .data : ALIGN(4K)
    {
        data_start = .;
        *(.data)
        data_end = .;
    }

    .bss : ALIGN(4K)
    {
        bss_start = .;
        *(COMMON)
        *(.bss)
        *(.stack)
        bss_end = .;
        endkernel = .;
    }
}