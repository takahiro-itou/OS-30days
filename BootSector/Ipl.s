
##  hello-os
##  TAB=4

    .EQU    CYLS,   10
    .EQU    ADDR,   0xC200

.code16
.text

/*  以下は標準的な FAT 12 フォーマットフロッピーディスクのための記述    */

    .byte   0xeb, 0x4e, 0x90
    .ascii  "HARIBOTE"      /*  ブートセクタの名前を自由に書いてよい    */
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
    .ascii  "HARIBOTEOS "   /*  ディスクの名前 (11バイト)       */
    .ascii  "FAT12   "      /*  フォーマットの名前 ( 8バイト)   */
    .skip   18, 0x00        /*  とりあえず 18 バイトあけておく  */

/*  プログラム本体  */
entry:
    MOVW    $0,     %AX     /*  レジスタ初期化  */
    MOVW    %AX,    %SS
    MOVW    $0x7c00,    %SP
    MOVW    %AX,    %DS

/* ディスクを読む。 */
    MOVW    $0x0820,    %AX
    MOVW    %AX,    %ES

    MOVB    $0,     %CH     /*  シリンダ 0  */
    MOVB    $0,     %DH     /*  ヘッド 0    */
    MOVB    $2,     %CL     /*  セクタ 2    */

readloop:
    MOVW    $0,     %SI     /*  失敗回数を数えるレジスタ。  */
retry:
    MOVB    $0x02,  %AH     /*  AH=0x02:ディスク読み込み。  */
    MOVB    $0x01,  %AL     /*  1 セクタ    */
    MOVW    $0,     %BX
    MOVB    $0x00,  %DL     /*  A ドライブ  */
    INT     $0x13           /*  ディスク BIOS 呼び出し  */
    JNC     next            /*  エラーがおきなければ next へ。  */
    ADDW    $1,     %SI     /*  SI に 1 を足す  */
    CMPW    $5,     %SI     /*  SI と 5 を比較  */
    JAE     error           /*  SI >= 5 だったら error  へ  */

    MOVB    $0x00,  %AH
    MOVB    $0x00,  %DL     /*  A ドライブ  */
    INT     $0x13           /*  ドライブのリセット  */
    JMP     retry
next:
    MOVW    %ES,    %AX     /*  アドレスを 0x0200 進める。  */
    ADDW    $0x0020,    %AX
    MOVW    %AX,    %ES
    ADDB    $1,     %CL     /*  CL に 1 を足す  */
    CMPB    $18,    %CL
    JBE     readloop

    MOVB    $1,     %CL
    ADDB    $1,     %DH     /*  ヘッド+1    */
    CMPB    $2,     %DH
    JB      readloop

    MOVB    $0,     %DH
    ADDB    $1,     %CH     /*  シリンダ+1  */
    CMPB    $CYLS,  %CH
    JB      readloop

/*  読み終わったので haribote.sys を実行。  */
    JMP     0xC200

error:
    MOV     $msg,   %SI
putloop:
    MOVB    (%SI),  %AL
    ADDW    $1,     %SI     /*  SI に 1 を足す  */
    CMPB    $0,     %AL
    JE      fin
    MOVB    $0x0e,  %AH     /*  壱文字表示ファンクション    */
    MOVW    $15,    %BX     /*  カラーコード    */
    INT     $0x10           /*  ビデオ BIOS 呼び出し    */
    JMP     putloop
fin:
    HLT                     /*  何かあるまで CPU  を停止させる  */
    JMP     fin             /*  無限ループ  */

/*  メッセージ部分  */
.data
msg:
    .byte   0x0a, 0x0a      /*  改行を２つ  */
    .ascii  "load error"
    .byte   0x0a            /*  改行    */
    .byte   0
