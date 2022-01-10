
.code32
.globl      api_settimer

.text

api_settimer:
    PUSHL   %EBX
    MOVL    $18,    %EDX
    MOVL     8(%ESP),   %EBX
    MOVL    12(%ESP),   %EAX
    INT     $0x40
    POPL    %EBX
    RET
