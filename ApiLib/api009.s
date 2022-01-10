
.code32
.globl      api_malloc

.text

api_malloc:
    PUSHL   %EBX
    MOVL    $9,     %EDX
    MOVL    %CS:(0x0020),   %EBX
    MOVL    8(%ESP),    %ECX
    INT     $0x40
    POPL    %EBX
    RET
