
.code32
.globl      api_point
.globl      api_refreshwin
.globl      api_linewin
.globl      api_closewin
.globl      api_getkey
.globl      api_alloctimer
.globl      api_inittimer
.globl      api_settimer
.globl      api_freetimer
.globl      api_beep

.text

api_point:
    PUSHL   %EDI
    PUSHL   %ESI
    PUSHL   %EBX
    MOVL    $11,    %EDX
    MOVL    16(%ESP),   %EBX    # win
    MOVL    20(%ESP),   %ESI    # x
    MOVL    24(%ESP),   %EDI    # y
    MOVL    28(%ESP),   %EAX    # col
    INT     $0x40
    POPL    %EBX
    POPL    %ESI
    POPL    %EDI
    RET

api_refreshwin:
    PUSHL   %EDI
    PUSHL   %ESI
    PUSHL   %EBX
    MOVL    $12,    %EDX
    MOVL    16(%ESP),   %EBX    # win
    MOVL    20(%ESP),   %EAX    # x0
    MOVL    24(%ESP),   %ECX    # y0
    MOVL    28(%ESP),   %ESI    # x1
    MOVL    32(%ESP),   %EDI    # y2
    INT     $0x40
    POPL    %EBX
    POPL    %ESI
    POPL    %EDI
    RET

api_linewin:
    PUSHL   %EDI
    PUSHL   %ESI
    PUSHL   %EBP
    PUSHL   %EBX
    MOVL    $13,    %EDX
    MOVL    20(%ESP),   %EBX    # win
    MOVL    24(%ESP),   %EAX    # x0
    MOVL    28(%ESP),   %ECX    # y0
    MOVL    32(%ESP),   %ESI    # x1
    MOVL    36(%ESP),   %EDI    # y1
    MOVL    40(%ESP),   %EBP    # col
    INT     $0x40
    POPL    %EBX
    POPL    %EBP
    POPL    %ESI
    POPL    %EDI
    RET

api_closewin:
    PUSHL   %EBX
    MOVL    $14,    %EDX
    MOVL    8(%ESP),    %EBX    # win
    INT     $0x40
    POPL    %EBX
    RET

api_getkey:
    MOVL    $15,    %EDX
    MOVL    4(%ESP),    %EAX    # mode
    INT     $0x40
    RET

api_alloctimer:
    MOVL    $16,    %EDX
    INT     $0x40
    RET

api_inittimer:
    PUSHL   %EBX
    MOVL    $17,    %EDX
    MOVL     8(%ESP),   %EBX
    MOVL    12(%ESP),   %EAX
    INT     $0x40
    POPL    %EBX
    RET

api_settimer:
    PUSHL   %EBX
    MOVL    $18,    %EDX
    MOVL     8(%ESP),   %EBX
    MOVL    12(%ESP),   %EAX
    INT     $0x40
    POPL    %EBX
    RET

api_freetimer:
    PUSHL   %EBX
    MOVL    $19,    %EDX
    MOVL    8(%ESP),    %EBX
    INT     $0x40
    POPL    %EBX
    RET

api_beep:
    MOVL    $20,    %EDX
    MOVL    4(%ESP),    %EAX
    INT     $0x40
    RET

