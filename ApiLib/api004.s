
.code32
.globl      api_end

.text

api_end:        # void api_end(void)
    MOVL    $4,     %EDX
    INT     $0x40
