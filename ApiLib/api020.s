
.code32
.globl      api_beep

.text

api_beep:
    MOVL    $20,    %EDX
    MOVL    4(%ESP),    %EAX
    INT     $0x40
    RET
