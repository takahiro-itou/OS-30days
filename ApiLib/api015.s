
.code32
.globl      api_getkey

.text

api_getkey:
    MOVL    $15,    %EDX
    MOVL    4(%ESP),    %EAX    # mode
    INT     $0x40
    RET
