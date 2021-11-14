
.code32

    MOVB    'A',    %AL
    CALL    0x0bb8
fin:
    HLT
    JMP     fin
