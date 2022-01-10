
.code32
.globl      api_freetimer

.text

api_freetimer:
    PUSHL   %EBX
    MOVL    $19,    %EDX
    MOVL    8(%ESP),    %EBX
    INT     $0x40
    POPL    %EBX
    RET
