
.code32
.globl      api_putchar
.globl      api_putstr0
.globl      api_end
.globl      api_openwin

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

api_openwin:    # int api_openwin(char *buf, int xsiz, int ysiz,
                #                 int col_inv, char *title)
    PUSHL   %EDI
    PUSHL   %ESI
    PUSHL   %EBX
    MOVL    $5,     %EDX
    MOVL    16(%ESP),   %EBX    # buf
    MOVL    20(%ESP),   %ESI    # xsiz
    MOVL    24(%ESP),   %EDI    # ysiz
    MOVL    28(%ESP),   %EAX    # col_inv
    MOVL    32(%ESP),   %ECX    # title
    INT     $0x40
    POPL    %EBX
    POPL    %ESI
    POPL    %EDI
    RET
