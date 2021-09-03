
##  hello-os
##  TAB=4

    .code16
    .org    0x0000

/*  以下は標準的な FAT 12 フォーマットフロッピーディスクのための記述    */

    .byte   0xeb, 0x4e, 0x90
    .ascii  "HELLOIPL"      /*  ブートセクタの名前を自由に書いてよい    */
    .word   512             /*  １セクタの大きさ    */
    .byte   1               /*  クラスタの大きさ    */
    .word   1               /*  FAT がどこから始まるか  */
    .byte   2               /*  FAT の個数          */
    .word   224             /*  ルートディレクトリ領域の大きさ  */
    .word   2880            /*  このドライブの大きさ 2880 セクタ    */
    .byte   0xf0            /*  メディアのタイプ    */
    .word   9               /*  FAT 領域の長さ      */
    .word   18              /*  トラックにいくつのセクタがあるか    */
    .word   2               /*  ヘッドの数          */
    .long   0               /*  パーティションを使ってないのでここは必ず 0  */
    .long   2880            /*  このドライブの大きさをもう一度書く  */
    .byte   0, 0, 0x29      /*  よく分からないけどこの値にしておく  */
    .long   0xffffffff      /*  たぶんボリュームシリアル番号    */
    .ascii  "HELLO-OS   "   /*  ディスクの名前 (11バイト)       */
    .ascii  "FAT12   "      /*  フォーマットの名前 ( 8バイト)   */
    .skip   18, 0x00        /*  とりあえず 18 バイトあけておく  */

/*  プログラム本体  */
entry:
    movw    $0,     %ax     /*  レジスタ初期化  */
    movw    %ax,    %ss
    movw    $0x7c00,    %sp
    movw    %ax,    %ds

/* ディスクを読む。 */
    movw     $0x0820,    %ax
    movw    %ax,    %es

    movb    $0,     %ch     /*  シリンダ 0  */
    movb    $0,     %dh     /*  ヘッド 0    */
    movb    $2,     %cl     /*  セクタ 2    */

readloop:
    movw    $0,     %si     /*  失敗回数を数えるレジスタ。  */
retry:
    movb    $0x02,  %ah     /*  AH=0x02:ディスク読み込み。  */
    movb    $0x01,  %al     /*  1 セクタ    */
    movw    $0,     %bx
    movb    $0x00,  %dl     /*  A ドライブ  */
    int     $0x13           /*  ディスク BIOS 呼び出し  */
    jnc     next            /*  エラーがおきなければ next へ。  */
    addw    $1,     %si     /*  SI に 1 を足す  */
    cmpw    $5,     %si     /*  SI と 5 を比較  */
    jae     error           /*  SI >= 5 だったら error  へ  */

    movb    $0x00,  %ah
    movb    $0x00,  %dl     /*  A ドライブ  */
    int     $0x13           /*  ドライブのリセット  */
    jmp     retry
next:
    movw    %es,    %ax     /*  アドレスを 0x0200 進める。  */
    addw    $0x0020,    %ax
    movw    %ax,    %es
    addb    $1,     %cl     /*  CL に 1 を足す  */
    cmpb    $18,    %cl
    jbe     readloop
fin:
    hlt                     /*  何かあるまで CPU  を停止させる  */
    jmp     fin             /*  無限ループ  */

error:
    mov     $msg,   %si
putloop:
    movb    (%si),  %al
    addw    $1,     %si     /*  SI に 1 を足す  */
    cmpb    $0,     %al
    je      fin
    movb    $0x0e,  %ah     /*  壱文字表示ファンクション    */
    movw    $15,    %bx     /*  カラーコード    */
    int     $0x10           /*  ビデオ BIOS 呼び出し    */
    jmp     putloop

/*  メッセージ部分  */
.data
msg:
    .byte   0x0a, 0x0a      /*  改行を２つ  */
    .ascii  "load error"
    .byte   0x0a            /*  改行    */
    .byte   0
