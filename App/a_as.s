
.code32
.globl      api_putchar
.globl      api_putstr0
.globl      api_end
.globl      api_openwin
.globl      api_putstrwin
.globl      api_boxfilwin
.globl      api_initmalloc
.globl      api_malloc
.globl      api_free
.globl      api_point
.globl      api_refreshwin
.globl      api_linewin
.globl      api_closewin
.globl      api_getkey
.globl      api_alloctimer
.globl      api_inittimer
.globl      api_settimer
.globl      api_freetimer

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

api_putstrwin:
    PUSHL   %EDI
    PUSHL   %ESI
    PUSHL   %EBP
    PUSHL   %EBX
    MOVL    $6,     %EDX
    MOVL    20(%ESP),   %EBX    # win
    MOVL    24(%ESP),   %ESI    # x
    MOVL    28(%ESP),   %EDI    # y
    MOVL    32(%ESP),   %EAX    # col
    MOVL    36(%ESP),   %ECX    # len
    MOVL    40(%ESP),   %EBP    # str
    INT     $0x40
    POPL    %EBX
    POPL    %EBP
    POPL    %ESI
    POPL    %EDI
    RET

api_boxfilwin:
    PUSHL   %EDI
    PUSHL   %ESI
    PUSHL   %EBP
    PUSHL   %EBX
    MOVL    $7,     %EDX
    MOVL    20(%ESP),   %EBX    # win
    MOVL    24(%ESP),   %EAX    # x0
    MOVL    28(%ESP),   %ECX    # y0
    MOVL    32(%ESP),   %ESI    # x1
    MOVL    36(%ESP),   %EDI    # y1
    MOVL    40(%ESP),   %EBP    # col
    INT     $0x40
    POPL    %EBX
    POPL    %EBP
    POPL    %ESI
    POPL    %EDI
    RET

api_initmalloc:
    PUSHL   %EBX
    MOVL    $8,     %EDX
    MOVL    %CS:(0x0020),   %EBX
    MOVL    %EBX,   %EAX
    ADDL    $32*1024,   %EAX
    MOVL    %CS:(0x0000),   %ECX
    SUBL    %EAX,   %ECX
    INT     $0x40
    POPL    %EBX
    RET

api_malloc:
    PUSHL   %EBX
    MOVL    $9,     %EDX
    MOVL    %CS:(0x0020),   %EBX
    MOVL    8(%ESP),    %ECX
    INT     $0x40
    POPL    %EBX
    RET

api_free:
    PUSHL   %EBX
    MOVL    $10,    %EDX
    MOVL    %CS:(0x0020),   %EBX
    MOVL     8(%ESP),   %EAX
    MOVL    12(%ESP),   %ECX
    INT     $0x40
    POPL    %EBX
    RET

api_point:
    PUSHL   %EDI
    PUSHL   %ESI
    PUSHL   %EBX
    MOVL    $11,    %EDX
    MOVL    16(%ESP),   %EBX    # win
    MOVL    20(%ESP),   %ESI    # x
    MOVL    24(%ESP),   %EDI    # y
    MOVL    28(%ESP),   %EAX    # col
    INT     $0x40
    POPL    %EBX
    POPL    %ESI
    POPL    %EDI
    RET

api_refreshwin:
    PUSHL   %EDI
    PUSHL   %ESI
    PUSHL   %EBX
    MOVL    $12,    %EDX
    MOVL    16(%ESP),   %EBX    # win
    MOVL    20(%ESP),   %EAX    # x0
    MOVL    24(%ESP),   %ECX    # y0
    MOVL    28(%ESP),   %ESI    # x1
    MOVL    32(%ESP),   %EDI    # y2
    INT     $0x40
    POPL    %EBX
    POPL    %ESI
    POPL    %EDI
    RET

api_linewin:
    PUSHL   %EDI
    PUSHL   %ESI
    PUSHL   %EBP
    PUSHL   %EBX
    MOVL    $13,    %EDX
    MOVL    20(%ESP),   %EBX    # win
    MOVL    24(%ESP),   %EAX    # x0
    MOVL    28(%ESP),   %ECX    # y0
    MOVL    32(%ESP),   %ESI    # x1
    MOVL    36(%ESP),   %EDI    # y1
    MOVL    40(%ESP),   %EBP    # col
    INT     $0x40
    POPL    %EBX
    POPL    %EBP
    POPL    %ESI
    POPL    %EDI
    RET

api_closewin:
    PUSHL   %EBX
    MOVL    $14,    %EDX
    MOVL    8(%ESP),    %EBX    # win
    INT     $0x40
    POPL    %EBX
    RET

api_getkey:
    MOVL    $15,    %EDX
    MOVL    4(%ESP),    %EAX    # mode
    INT     $0x40
    RET

api_alloctimer:
    MOVL    $16,    %EDX
    INT     $0x40
    RET

api_inittimer:
    PUSHL   %EBX
    MOVL    $17,    %EDX
    MOVL     8(%ESP),   %EBX
    MOVL    12(%ESP),   %EAX
    INT     $0x40
    POPL    %EBX
    RET

api_settimer:
    PUSHL   %EBX
    MOVL    $18,    %EDX
    MOVL     8(%ESP),   %EBX
    MOVL    12(%ESP),   %EAX
    INT     $0x40
    POPL    %EBX
    RET

api_freetimer:
    PUSHL   %EBX
    MOVL    $19,    %EDX
    MOVL    8(%ESP),    %EBX
    INT     $0x40
    POPL    %EBX
    RET
