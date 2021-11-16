
.code32
.globl      io_hlt
.globl      io_cli, io_sti, io_stihlt
.globl      io_in8, io_in16, io_in32
.globl      io_out8, io_out16, io_out32
.globl      io_load_eflags, io_store_eflags
.globl      load_gdtr, load_idtr
.globl      load_cr0, store_cr0
.globl      load_tr
.globl      asm_inthandler20
.globl      asm_inthandler21
.globl      asm_inthandler27
.globl      asm_inthandler2c
.globl      memtest_sub
.globl      farjmp
.globl      farcall
.globl      asm_hrb_api, start_app
.extern     inthandler20, inthandler21, inthandler27, inthandler2c
.extern     hrb_api

.text

io_hlt:         # void io_hlt(void)
    HLT
    RET

io_cli:         # void io_cli(void)
    CLI
    RET

io_sti:         # void io_sti(void)
    STI
    RET

io_stihlt:      # void io_stihlt(void)
    STI
    HLT
    RET

io_in8:         # int io_in8(int port)
    MOVL    4(%ESP),    %EDX
    MOVL    $0,     %EAX
    INB     %DX,    %AL
    RET

io_in16:        # int io_in16(int port)
    MOVL    4(%ESP),    %EDX
    MOVL    $0,     %EAX
    INW     %DX,    %AX
    RET

io_in32:        # int io_in32(int port)
    MOVL    4(%ESP),    %EDX
    INL     %DX,    %EAX
    RET

io_out8:        # void io_out8(int port, int data)
    MOVL    4(%ESP),    %EDX
    MOVB    8(%ESP),    %AL
    OUTB    %AL,    %DX
    RET

io_out16:       # void io_out16(int port, int data)
    MOVL    4(%ESP),    %EDX
    MOVL    8(%ESP),    %EAX
    OUTW    %AX,    %DX
    RET

io_out32:       # void io_out32(int port, int data)
    MOVL    4(%ESP),    %EDX
    MOVL    8(%ESP),    %EAX
    OUTL    %EAX,   %DX
    RET

io_load_eflags:     # int io_load_eflags(void)
    PUSHFL
    POPL    %EAX
    RET

io_store_eflags:    # void io_store_eflags(int eflags)
    MOVL    4(%ESP),    %EAX
    PUSH    %EAX
    POPFL
    RET

load_gdtr:          # void load_gdtr(int limit, int addr)
    MOVW    4(%ESP),    %AX     /*  limit   */
    MOVW    %AX,    6(%ESP)
    LGDT    6(%ESP)
    RET

load_idtr:          # void load_idtr(int limit, int addr)
    MOVW    4(%ESP),    %AX     /*  limit   */
    MOVW    %AX,    6(%ESP)
    LIDT    6(%ESP)
    RET


load_cr0:           # int load_cr0(void)
    MOV     %CR0,   %EAX
    RET

store_cr0:          # void store_cr0(int cr0)
    MOVL    4(%ESP),    %EAX
    MOVL    %EAX,       %CR0
    RET

load_tr:            # void load_tr(int tr)
    LTR     4(%ESP)
    RET

asm_inthandler20:
    PUSHW   %ES
    PUSHW   %DS
    PUSHA
    MOVL    %ESP,   %EAX
    PUSHL   %EAX
    MOVW    %SS,    %AX
    MOVW    %AX,    %DS
    MOVW    %AX,    %ES
    CALL    inthandler20
    POP     %EAX
    POPA
    POPW    %DS
    POPW    %ES
    IRET

asm_inthandler21:
    PUSHW   %ES
    PUSHW   %DS
    PUSHA
    MOVL    %ESP,   %EAX
    PUSHL   %EAX
    MOVW    %SS,    %AX
    MOVW    %AX,    %DS
    MOVW    %AX,    %ES
    CALL    inthandler21
    POP     %EAX
    POPA
    POPW    %DS
    POPW    %ES
    IRET

asm_inthandler27:
    PUSHW   %ES
    PUSHW   %DS
    PUSHA
    MOVL    %ESP,   %EAX
    PUSHL   %EAX
    MOVW    %SS,    %AX
    MOVW    %AX,    %DS
    MOVW    %AX,    %ES
    CALL    inthandler27
    POP     %EAX
    POPA
    POPW    %DS
    POPW    %ES
    IRET

asm_inthandler2c:
    PUSHW   %ES
    PUSHW   %DS
    PUSHA
    MOVL    %ESP,   %EAX
    PUSHL   %EAX
    MOVW    %SS,    %AX
    MOVW    %AX,    %DS
    MOVW    %AX,    %ES
    CALL    inthandler2c
    POP     %EAX
    POPA
    POPW    %DS
    POPW    %ES
    IRET


memtest_sub:
    PUSHL   %EDI
    PUSHL   %ESI
    PUSHL   %EBX
    MOVL    $0xaa55aa55,    %ESI    # pat0 = 0xaa55aa55
    MOVL    $0x55aa55aa,    %EDI    # pat1 = 0x55aa55aa
    MOVL    16(%ESP),  %EAX         # i = start
mts_loop:
    MOVL    %EAX,   %EBX
    ADDL    $0xffc,  %EBX           # p = i + 0xffc
    MOVL    (%EBX), %EDX            # old = *p
    MOVL    %ESI,   (%EBX)          # *p = pat0
    XORL    $0xffffffff,    (%EBX)  # *p ^= 0xffffffff
    CMPL    (%EBX), %EDI            # if (*p != pat1) goto fin
    JNE     mts_fin
    XORL    $0xffffffff,    (%EBX)  # *p ^= 0xffffffff
    CMPL    (%EBX), %ESI            # if (*p != pat0) goto fin
    JNE     mts_fin
    MOVL    %EDX,   (%EBX)          # *p = old
    ADDL    $0x1000, %EAX           # i += 0x1000
    CMPL    20(%ESP),  %EAX         # if (i <= end) goto mts_loop
    JBE     mts_loop
    POPL    %EBX
    POPL    %ESI
    POPL    %EDI
    RET
mts_fin:
    MOVL    %EDX,   (%EBX)      # *p = old
    POPL    %EBX
    POPL    %ESI
    POPL    %EDI
    RET

farjmp:     # void farjmp(int eip, int cs)
    LJMPL   * 4(%ESP)
    RET

farcall:    # void farcall(int eip, int cs)
    LCALLL  * 4(%ESP)
    RET

asm_hrb_api:
    /*  都合のいいことに最初から割り込み禁止になっている。  */
    PUSHW   %DS
    PUSHW   %ES
    PUSHA   /*  保存のための PUSH           */
    MOVL    $1*8,   %EAX
    MOVW    %AX,    %DS
    MOVL    (0xfe4),    %ECX
    ADDL    $-40,       %ECX
    MOVL    %ESP,   32(%ECX)
    MOVW    %SS,    36(%ECX)

    MOVL    (%ESP),     %EDX
    MOVL     4(%ESP),   %EBX
    MOVL    %EDX,     (%ECX)
    MOVL    %EBX,    4(%ECX)

    MOVL     8(%ESP),   %EDX
    MOVL    12(%ESP),   %EBX
    MOVL    %EDX,    8(%ECX)
    MOVL    %EBX,   12(%ECX)

    MOVL    16(%ESP),   %EDX
    MOVL    20(%ESP),   %EBX
    MOVL    %EDX,   16(%ECX)
    MOVL    %EBX,   20(%ECX)

    MOVL    24(%ESP),   %EDX
    MOVL    28(%ESP),   %EBX
    MOVL    %EDX,   24(%ECX)
    MOVL    %EBX,   28(%ECX)

    MOVW    %AX,    %ES
    MOVW    %AX,    %SS
    MOVL    %ECX,   %ESP

    STI
    CALL    hrb_api

    MOVL    32(%ESP),   %ECX
    MOVL    36(%ESP),   %EAX
    CLI
    MOVW    %AX,    %SS
    MOVL    %ECX,   %ESP
    POPA
    POPW    %ES
    POPW    %DS
    IRET

start_app:      # void start_app(int eip, int cs, int esp, int ds)
    PUSHA   /*  レジスタを全部保存しておく  */
    MOVL    36(%ESP),   %EAX    /*  アプリ用の EIP  */
    MOVL    40(%ESP),   %ECX    /*  アプリ用の CS   */
    MOVL    44(%ESP),   %EDX    /*  アプリ用の ESP  */
    MOVL    48(%ESP),   %EBX    /*  アプリ用の DS   */
    MOVL    %ESP,   (0xfe4)     /*  OS用の ESP      */
    CLI
    MOVW    %BX,    %ES
    MOVW    %BX,    %SS
    MOVW    %BX,    %DS
    MOVW    %BX,    %FS
    MOVW    %BX,    %GS
    MOVL    %EDX,   %ESP
    STI
    PUSHL   %ECX
    PUSH    %EAX
    LCALLL  * (%ESP)

    /*  アプリが終了するとここに帰ってくる  */
    MOVL    $1*8,   %EAX
    CLI
    MOVW    %AX,    %ES
    MOVW    %AX,    %SS
    MOVW    %AX,    %DS
    MOVW    %AX,    %FS
    MOVW    %AX,    %GS
    MOVL    (0xfe4),    %ESP
    STI
    POPA
    RET
