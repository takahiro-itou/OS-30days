
.code32
.globl      api_boxfilwin

.text

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
