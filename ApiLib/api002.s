
.code32
.globl      api_putstr0

.text

api_putstr0:    # void api_putstr0(char *s)
    PUSHL   %EBX
    MOVL    $2,     %EDX
    MOVL    8(%ESP),    %EBX
    INT     $0x40
    POPL    %EBX
    RET
