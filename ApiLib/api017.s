
.code32
.globl      api_inittimer

.text

api_inittimer:
    PUSHL   %EBX
    MOVL    $17,    %EDX
    MOVL     8(%ESP),   %EBX
    MOVL    12(%ESP),   %EAX
    INT     $0x40
    POPL    %EBX
    RET
