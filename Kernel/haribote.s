
.code16
.text

    /*  BOOT_INFO 関係  */
    .equ    CYLS,   0x0FF0      /*  ブートセクタが設定する  */
    .equ    LEDS,   0x0FF1
    .equ    VMODE,  0x0FF2      /*  色数に関する情報。何ビットカラーか  */
    .equ    SCRNX,  0x0FF4      /*  解像度の X (screen x)   */
    .equ    SCRNY,  0x0FF6      /*  解像度の Y (screen y)   */
    .equ    VRAM,   0x0FF8      /*  グラフィックバッファの開始番地  */

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

fin:
    HLT
    JMP     fin
