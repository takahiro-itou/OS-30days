
.code32

    MOVB    $'h',   %AL
    INT     $0x40
    MOVB    $'e',   %Al
    INT     $0x40
    MOVB    $'l',   %Al
    INT     $0x40
    MOVB    $'l',   %Al
    INT     $0x40
    MOVB    $'o',   %Al
    INT     $0x40

    LRET
