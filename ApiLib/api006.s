
.code32
.globl      api_putstrwin

.text

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
