
.code32

    MOVB    $'A',   %AL
    LCALLL  $2*8, $0x0bbd
    LRET
