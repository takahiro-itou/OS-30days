
.code32
.globl      api_fread

.text

api_fread:
    PUSHL   %EBX
    MOVL    $25,    %EDX
    MOVL    16(%ESP),   %EAX
    MOVL    12(%ESP),   %ECX
    MOVL     8(%ESP),   %EBX
    INT     $0x40
    POPL    %EBX
    RET
