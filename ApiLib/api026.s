
.code32
.globl      api_cmdline

.text

api_fread:
    PUSHL   %EBX
    MOVL    $26,    %EDX
    MOVL    12(%ESP),   %ECX
    MOVL     8(%ESP),   %EBX
    INT     $0x40
    POPL    %EBX
    RET
