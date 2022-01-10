
.code32
.globl      api_alloctimer
.globl      api_inittimer
.globl      api_settimer
.globl      api_freetimer
.globl      api_beep

.text

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

