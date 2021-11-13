
OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)

SECTIONS {
    .  = 0;
    .text       : { *(.text) }
    .data       : { *(.data) }
    .rodata     : {
        *(.rodata);
        *(.rodata.*);
    }
}
