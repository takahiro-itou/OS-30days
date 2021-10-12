
OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)

ENTRY(HariMain)

BOOT_PACK_BASE  = 0x00280000;
SEGMENT_BASE    = 0x00280000;

SECTIONS {
    .  =  BOOT_PACK_BASE - SEGMENT_BASE;
    .header : {
        LONG(0x00314000);
        LONG(0x69726148);
        LONG(0x00000000);
        LONG(0x00310000);
        LONG(0x000011a8);
        LONG(0x000010c8);
        LONG(0xe9000000);
        LONG(HariMain - . - 4);
        LONG(0x00313bf0);
    }
    .text       : { *(.text) }
    .data       : { *(.data) }
}
