
.code32

    MOVB    $'h',   %AL
    LCALLL  $2*8, $0x0bbd
    MOVB    $'e',   %Al
    LCALLL  $2*8, $0x0bbd
    MOVB    $'l',   %Al
    LCALLL  $2*8, $0x0bbd
    MOVB    $'l',   %Al
    LCALLL  $2*8, $0x0bbd
    MOVB    $'o',   %Al
    LCALLL  $2*8, $0x0bbd
    LRET
