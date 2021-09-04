
OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)

BOOT_SECTOR_BASE    = 0xC200;

SECTIONS {
    .  =  BOOT_SECTOR_BASE;
    .text       : { *(.text) }
    .data       : { *(.data) }
}
