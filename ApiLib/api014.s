
.code32
.globl      api_closewin

.text

api_closewin:
    PUSHL   %EBX
    MOVL    $14,    %EDX
    MOVL    8(%ESP),    %EBX    # win
    INT     $0x40
    POPL    %EBX
    RET
