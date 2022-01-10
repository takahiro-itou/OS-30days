
.code32
.globl      api_initmalloc

.text

api_initmalloc:
    PUSHL   %EBX
    MOVL    $8,     %EDX
    MOVL    %CS:(0x0020),   %EBX
    MOVL    %EBX,   %EAX
    ADDL    $32*1024,   %EAX
    MOVL    %CS:(0x0000),   %ECX
    SUBL    %EAX,   %ECX
    INT     $0x40
    POPL    %EBX
    RET
