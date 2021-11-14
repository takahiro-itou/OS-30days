
.code32

    MOVL    $msg,   %ECX
    MOVL    $1,     %EDX
putloop:
    MOVB    %CS:(%ECX), %AL
    CMPB    $0,     %AL
    JE      fin
    INT     $0x40
    ADDL    $1,     %ECX
    JMP     putloop
fin:
    LRET

msg:
    .ascii  "hello"
    .byte   0
