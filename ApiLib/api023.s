
.code32
.globl      api_fseek

.text

api_fopen:
    PUSHL   %EBX
    MOVL    $23,    %EDX
    MOVL     8(%ESP),   %EAX
    MOVL    16(%ESP),   %ECX
    MOVL    12(%ESP),   %EBX
    INT     $0x40
    POPL    %EBX
    RET
