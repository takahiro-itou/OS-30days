
.code32

    MOVB    $'A',   %AL
    LCALL   $2*8, $0x0bb8
fin:
    HLT
    JMP     fin
