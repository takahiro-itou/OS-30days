
.code32
.globl      api_getlang

.text

api_getlang:
    MOVL    $27,    %EDX
    INT     $0x40
    RET
