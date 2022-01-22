
.code32
.globl      api_fopen

.text

api_fopen:
    PUSHL   %EBX
    MOVL    $21,    %EDX
    MOVL    8(%ESP),    %EBX
    INT     $0x40
    POPL    %EBX
    RET
