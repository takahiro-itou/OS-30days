
.code32
.globl      api_settimer
.globl      api_freetimer
.globl      api_beep

.text

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

