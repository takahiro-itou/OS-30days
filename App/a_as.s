
.code32
.globl      api_putchar

.text

api_putchar:    # void api_putchar(int c)
    MOVL    $1,     %EDX
    MOVB    4(%ESP),    %AL
    INT     $0x40
    RET
