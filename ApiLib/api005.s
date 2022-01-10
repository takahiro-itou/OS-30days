
.code32
.globl      api_openwin

.text

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
