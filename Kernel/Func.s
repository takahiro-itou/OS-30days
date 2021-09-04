
.code32
.global     io_hlt
.global     write_mem8

.text

io_hlt:         # void io_hlt(void)
    HLT
    RET

write_mem8:     # void write_mem8(int addr, int data)
    MOVL    4(%ESP),    %ECX
    MOVB    8(%ESP),    %AL
    MOVB    %AL,    (%ECX)
    RET
