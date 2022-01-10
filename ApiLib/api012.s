
.code32
.globl      api_refreshwin

.text

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
