
.equ    BOTPAK,     0x00280000      /*  bootpackのロード先  */
.equ    DSKCAC,     0x00100000      /*  ディスクキャッシュの場所    */
.equ    DSKCAC0,    0x00008000      /*  ディスクキャッシュ(リアルモード)  */

/*  BOOT_INFO 関係  */
.equ    CYLS,   0x0FF0      /*  ブートセクタが設定する  */
.equ    LEDS,   0x0FF1
.equ    VMODE,  0x0FF2      /*  色数に関する情報。何ビットカラーか  */
.equ    SCRNX,  0x0FF4      /*  解像度の X (screen x)   */
.equ    SCRNY,  0x0FF6      /*  解像度の Y (screen y)   */
.equ    VRAM,   0x0FF8      /*  グラフィックバッファの開始番地  */

.code16
.text

    /*  画面モードを設定。  */
    MOVB    $0x13,  %AL     /*  VGA グラフィックス、320x00x8bit カラー  */
    MOVB    $0x00,  %AH
    INT     $0x10

    MOVB    $8,     (VMODE)     /*  画面モードをメモする。  */
    MOVW    $320,   (SCRNX)
    MOVW    $200,   (SCRNY)
    MOVL    $0x000a0000,    (VRAM)

    /*  キーボードの LED  状態を BIOS に教えてもらう。  */
    MOVB    $0x02,  %AH
    INT     $0x16
    MOVB    %AL,    (LEDS)

    /*  PIC が一切の割り込みを受け付けないようにする
        AT互換機の仕様では、PIC の初期化をするなら、
        こいつを CLI  前にやっておかないと、たまにハングアップする
        PIC の初期化は後でやる。    */
    MOVB    $0xff,  %AL
    OUTB    %AL,    $0x21
    NOP         /*  OUT 命令を連続させるとうまくいかない機種があるらしい    */
    OUTB    %AL,    $0xa1

    CLI

    /*  CPU から 1 MB 以上のメモリにアクセスできるように A20 GATE を設定    */
    CALL    waitkbout
    MOVB    $0xd1,  %AL
    OUT     %AL,    $0x64
    CALL    waitkbout
    MOVB    $0xdf,  %AL     /*  enable A20  */
    OUTB    %AL,    $0x60
    CALL    waitkbout

##################################################################

    /*  プロテクトモード移行。  */
.arch   i486
    LGDTL   (GDTR0)
    MOVL    %CR0,   %EAX
    ANDL    $0x7fffffff,    %EAX    /*  bit31を0にする。ページング禁止      */
    ORL     $0x00000001,    %EAX    /*  bit00を1にする。プロテクトモード    */
    MOVL    %EAX,   %CR0
    JMP     pipelineflush
pipelineflush:
    MOVW    $1*8,   %AX     /*  書き込み可能セグメント  */
    MOVW    %AX,    %DS
    MOVW    %AX,    %ES
    MOVW    %AX,    %FS
    MOVW    %AX,    %GS
    MOVW    %AX,    %SS

    /*  bootpackの転送  */
    MOVL    $bootpack,  %ESI    /*  転送元  */
    MOVL    $BOTPAK,    %EDI    /*  転送先  */
    MOVL    $512*1024/4,    %ECX
    CALL    memcpy

    /*  ついでにディスクデータも本来の位置へ転送    */
    MOVL    $0x7c00,    %ESI
    MOVL    $DSKCAC,    %EDI
    MOVL    $512/4,     %ECX

    MOVL    $DSKCAC0+512,   %ESI
    MOVL    $DSKCAC+512,    %EDI
    MOVL    $0,     %ECX
    MOVB    (CYLS), %CL
    IMULL   $512*18*2/4,    %ECX    /*  シリンダ数からバイト数/4に変換  */
    SUBL    $512/4, %ECX    /*  IPL の分だけ差し引く    */
    CALL    memcpy

    /*  asmhead でしなければいけないことは全部し終わったので、
        あとは bootpack に任せる    */
    /*  bootpackの起動  */
    MOVL    $BOTPAK,    %EBX
    MOVL    $0x11a8,    %ECX        # MOVL    16(%EBX),   %ECX
    ADDL    $3,     %ECX
    SHRL    $2,     %ECX
    JZ      skip
    MOVL    $0x10c8,    %ESI        # MOVL    20(%EBX),   %ESI
    ADDL    %EBX,       %ESI
    MOVL    $0x00310000,    %EDI    # MOVL    12(%EBX),   %EDI
    CALL    memcpy

skip:
    MOVW    $1*8,   %AX     /*  書き込み可能セグメント  */
    MOVW    %AX,    %DS
    MOVW    %AX,    %ES
    MOVW    %AX,    %FS
    MOVW    %AX,    %GS
    MOVW    %AX,    %SS

    # MOVL    12(%EBX),   %ESP    /*  スタック初期値  */
    MOVL    $0x00310000,    %ESP    /*  スタック初期値  */
#    LJMPL   $2*8, $0x0028001b
    LJMPL   $4*8, $0x0000001b

##################################################################
##  function

waitkbout:
    INB     $0x64,  %AL
    ANDB    $0x02,  %AL
    JNZ     waitkbout
    RET

memcpy:
    MOVL    (%ESI), %EAX
    ADDL    $4,     %ESI
    MOVL    %EAX,   (%EDI)
    ADDL    $4,     %EDI
    SUBL    $1,     %ECX
    JNZ     memcpy
    RET

##################################################################
##  GDT

.align  16
GDT0:
    .skip   8, 0x00                 /*  ヌルセレクタ    */

    /*  全領域を含むシステムセグメント。    */
    .word   0xffff, 0x0000, 0x9200, 0x00cf  /*  読み書き可能セグメント  */
    .word   0xffff, 0x0000, 0x9a00, 0x00cf  /*  実行可能セグメント      */

    /*  カーネルを含むシステムセグメント。  */
    .word   0xffff, 0x0000, 0x9228, 0x0047  /*  読み書き可能セグメント  */
    .word   0xffff, 0x0000, 0x9a28, 0x0047  /*  実行可能セグメント      */

    .word   0x0000
GDTR0:
    .word   8*5-1
    .int    GDT0

.align  16
bootpack:
