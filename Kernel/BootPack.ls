
OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)

ENTRY(HariMain)

BOOT_PACK_BASE  = 0x00280000;


SECTIONS {
    .  =  BOOT_PACK_BASE;
    .text       : { *(.text) }
    .data       : { *(.data) }
}
