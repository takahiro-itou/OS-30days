
.code32
.globl      api_fsize

.text

api_fsize:
    MOVL    $24,    %EDX
    MOVL    4(%ESP),    %EAX
    MOVL    8(%ESP),    %ECX
    INT     $0x40
    RET
