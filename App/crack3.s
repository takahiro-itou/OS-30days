
.code32

    MOVB    $0x34,  %AL
    OUTB    %AL,    $0x43
    MOVB    $0xff,  %AL
    OUTB    %AL,    $0x40
    MOVB    $0xff,  %AL
    OUTB    %AL,    $0x40

    MOVL    $4,     %EDX
    INT     $0x40
