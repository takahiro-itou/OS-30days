
.code32

    MOVL    $2,     %EDX
    MOVL    $msg,   %EBX
    INT     $0x40
    LRET

msg:
    .ascii  "hello"
    .byte   0
