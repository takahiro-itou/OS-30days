
OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)

ENTRY(HariMain)

CODE_SEGMENT_BASE   = 0x00000000;
DATA_SEGMENT_BASE   = 0x00004000;

MEMORY {
    ROM(rx)  : ORIGIN = CODE_SEGMENT_BASE, LENGTH = 16K
    RAM(rwx) : ORIGIN = DATA_SEGMENT_BASE, LENGTH = 16K
};

SECTIONS {
    .  =  ORIGIN(ROM);
    .header : {
        LONG(0x00010000);
        LONG(0x69726148);
        LONG(0x00000000);
        LONG(DATA_SEGMENT_BASE);
        LONG(_END_DATA - _START_DATA);
        LONG(_OFFSET_DATA);
        LONG(0xe9000000);
        LONG(HariMain - . - 4);
        LONG(0x00000000);
    } > ROM

    .text       : {
        *(.text)
        . = ALIGN(16);
        _size_of_text = .;
    } > ROM

    _OFFSET_DATA    = .;
    .data       (DATA_SEGMENT_BASE) :
    {
        _START_DATA = .;
        *(.data)
        *(.data.*)
        . = ALIGN(16);
    } > RAM  AT > ROM

    .rodata     . : {
        *(.rodata);
        *(.rodata.*);
        . = ALIGN(16);
        _END_DATA   = .;
    } > RAM  AT > ROM

}
