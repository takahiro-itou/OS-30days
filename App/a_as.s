
.code32
.globl      api_putchar
.globl      api_putstr0
.globl      api_end

.text

api_putchar:    # void api_putchar(int c)
    MOVL    $1,     %EDX
    MOVB    4(%ESP),    %AL
    INT     $0x40
    RET

api_putstr0:    # void api_putstr0(char *s)
    PUSHL   %EBX
    MOVL    $2,     %EDX
    MOVL    8(%ESP),    %EBX
    INT     $0x40
    POPL    %EBX
    RET

api_end:        # void api_end(void)
    MOVL    $4,     %EDX
    INT     $0x40
