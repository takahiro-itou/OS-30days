
.code32
.globl      api_putchar
.globl      api_end

.text

api_putchar:    # void api_putchar(int c)
    MOVL    $1,     %EDX
    MOVB    4(%ESP),    %AL
    INT     $0x40
    RET

api_end:        # void api_end(void)
    MOVL    $4,     %EDX
    INT     $0x40
