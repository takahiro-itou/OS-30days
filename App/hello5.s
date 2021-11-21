
.code32
.globl      HariMain

.text

HariMain:
    MOVL    $2,     %EDX
    MOVL    $msg,   %EBX
    INT     $0x40

    MOVL    $4,     %EDX
    INT     $0x40

.data

msg:
    .ascii  "hello, world"
    .byte   0x0a, 0
