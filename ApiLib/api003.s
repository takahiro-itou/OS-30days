
.code32
.globl      api_putstr1

.text

api_putstr1:    # void api_putstr1(char *s, int l)
    PUSHL   %EBX
    MOVL    $3,     %EDX
    MOVL     8(%ESP),   %EBX
    MOVL    12(%ESP),   %ECX
    INT     $0x40
    POPL    %EBX
    RET
