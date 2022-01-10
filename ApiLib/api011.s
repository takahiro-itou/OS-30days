
.code32
.globl      api_point

.text

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
