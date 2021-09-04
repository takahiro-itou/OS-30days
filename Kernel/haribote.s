
    .code16

    MOVB    $0x13,  %AL     /*  VGA グラフィックス、320x00x8bit カラー  */
    MOVB    $0x00,  %AH
    INT     $0x01

fin:
    HLT
    JMP     fin
