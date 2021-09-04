
OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)

KERNEL_IMAGE_BASE   = 0xC200;

SECTIONS {
    .  =  KERNEL_IMAGE_BASE;
    .text       : { *(.text) }
    .data       : { *(.data) }
}
