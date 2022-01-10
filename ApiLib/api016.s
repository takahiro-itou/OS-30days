
.code32
.globl      api_alloctimer

.text

api_alloctimer:
    MOVL    $16,    %EDX
    INT     $0x40
    RET
