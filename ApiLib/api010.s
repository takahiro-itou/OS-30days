
.code32
.globl      api_free

.text

api_free:
    PUSHL   %EBX
    MOVL    $10,    %EDX
    MOVL    %CS:(0x0020),   %EBX
    MOVL     8(%ESP),   %EAX
    MOVL    12(%ESP),   %ECX
    INT     $0x40
    POPL    %EBX
    RET
