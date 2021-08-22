
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
    .byte   0xb8, 0x00, 0x00, 0x8e,  0xd0, 0xbc, 0x00, 0x7c
    .byte   0x8e, 0xd8, 0x8e, 0xc0,  0xbe, 0x74, 0x7c, 0x8a
    .byte   0x04, 0x83, 0xc6, 0x01,  0x3c, 0x00, 0x74, 0x09
    .byte   0xb4, 0x0e, 0xbb, 0x0f,  0x00, 0xcd, 0x10, 0xeb
    .byte   0xee, 0xf4, 0xeb, 0xfd

/*  メッセージ部分  */
    .byte   0x0a, 0x0a      /*  改行を２つ  */
    .ascii  "hello, world"
    .byte   0x0a            /*  改行    */
    .byte   0

    .org    0x1fe
    .byte   0x55, 0xaa

/*  以下はブートセクタ以外の部分の記述  */
    .byte   0xf0, 0xff, 0xff, 0x00,  0x00, 0x00, 0x00, 0x00
    .space  4600, 0x00
    .byte   0xf0, 0xff, 0xff, 0x00,  0x00, 0x00, 0x00, 0x00
    .space  1469432,    0x00

