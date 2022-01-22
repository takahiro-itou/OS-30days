
.code32
.globl      api_fclose

.text

api_fclose:
    MOVL    $22,    %EDX
    MOVL    4(%ESP),    %EAX
    INT     $0x40
    RET
