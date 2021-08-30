
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
    movw    %ax,    %es

    movw    $msg,   %si

putloop:
    movb    (%si),  %al
    addw    $1,     %si     /*  SI に 1 を足す  */
    cmpb    $0,     %al
    je      fin
    movb    $0x0e,  %ah     /*  壱文字表示ファンクション    */
    movw    $15,    %bx     /*  カラーコード    */
    int     $0x10           /*  ビデオ BIOS 呼び出し    */
    jmp     putloop

fin:
    hlt                     /*  何かあるまで CPU  を停止させる  */
    jmp     fin             /*  無限ループ  */

/*  メッセージ部分  */
.data
msg:
    .byte   0x0a, 0x0a      /*  改行を２つ  */
    .ascii  "hello, world"
    .byte   0x0a            /*  改行    */
    .byte   0
