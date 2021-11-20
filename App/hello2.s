
.code32

    MOVL    $2,     %EDX
    MOVL    $msg,   %EBX
    INT     $0x40
    MOVL    $4,     %EDX
    INT     $0x40

msg:
    .ascii  "hello"
    .byte   0
