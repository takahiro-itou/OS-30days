
.code32
.globl      api_getlang

.text

api_cmdline:
    MOVL    $27,    %EDX
    INT     $0x40
    RET
