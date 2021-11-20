
.code32

    MOVL    $1*8,   %EAX
    MOVW    %AX,    %DS
    MOVB    $0, (0x102600)
    MOVL    $4,     %EDX
    INT     $0x40


