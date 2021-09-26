/*  bootpack  のメイン  */

#include "BootPack.h"
#include "../Common/stdio.h"

void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *)(0xff0);
    char s[40], mcursor[256], keybuf[32], mousebuf[128];
    int mx, my, i;
    struct MOUSE_DEC mdec;

    init_gdtidt();
    init_pic();
    io_sti();

    fifo8_init(&keyfifo, sizeof(keybuf), keybuf);
    fifo8_init(&mousefifo, sizeof(mousebuf), mousebuf);
    io_out8(PIC0_IMR, 0xf9);    /*  PIC1とキーボードを許可  */
    io_out8(PIC1_IMR, 0xef);    /*  マウスを許可            */

    init_keyboard();

    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    snprintf(s, sizeof(s) - 1, "(%3d, %3d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

    enable_mouse(&mdec);

    for (;;) {
        io_cli();
        if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
            io_stihlt();
        } else if (fifo8_status(&keyfifo) != 0) {
            i = fifo8_get(&keyfifo);
            io_sti();
            snprintf(s, sizeof(s), "%02X", i);
            boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
            putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
        } else if (fifo8_status(&mousefifo) != 0) {
            i = fifo8_get(&mousefifo);
            io_sti();
            if (mouse_decode(&mdec, i) != 0) {
                /*  データが揃ったので表示  */
                snprintf(s, sizeof(s), "[lcr %4d %4d]", mdec.x, mdec.y);
                if ((mdec.btn & 0x01) != 0) {
                    s[1] = 'L';
                }
                if ((mdec.btn & 0x02) != 0) {
                    s[3] = 'R';
                }
                if ((mdec.btn & 0x04) != 0) {
                    s[2] = 'C';
                }
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484,
                         32, 16, 32 + 15 * 8 - 1, 31);
                putfonts8_asc(binfo->vram, binfo->scrnx,
                              32, 16, COL8_FFFFFF, s);
                /*  マウスカーソルの移動。  */
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484,
                         mx, my, mx + 15, my + 15);     /*  マウス消す  */
                mx += mdec.x;
                my += mdec.y;
                if (mx < 0) {
                    mx = 0;
                }
                if (my < 0) {
                    my = 0;
                }
                if (mx > binfo->scrnx - 16) {
                    mx = binfo->scrnx - 16;
                }
                if (my > binfo->scrny - 16) {
                    my = binfo->scrny - 16;
                }
                snprintf(s, sizeof(s) - 1, "(%03d, %03d)", mx, my);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
                putblock8_8(binfo->vram, binfo->scrnx, 16, 16,
                            mx, my, mcursor, 16);   /*  マウス描く  */
            }
        }
    }
}

void wait_KBC_sendready(void)
{
    /*  キーボードコントローラがデータ送信可能になるのを待つ。  */
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
    return;
}

void init_keyboard(void)
{
    /*  キーボードコントローラの初期化  */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}

void enable_mouse(struct MOUSE_DEC *mdec)
{
    /*  マウス有効  */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);

    /*  うまくいくと ACK(0xfa)  が送信されてくる。  */
    mdec->phase = 0;    /*  マウスの 0xfa を待っている段階  */

    return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
    if (mdec->phase == 0) {
        /*  マウスの 0xfa を待っている段階  */
        if (dat == 0xfa) {
            mdec->phase = 1;
        }
        return ( 0 );
    }
    if (mdec->phase == 1) {
        /*  マウスの 1バイト目を待っている段階  */
        if ((dat & 0xc8) == 0x08) {
            /*  正しい 1バイト目だった  */
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return ( 0 );
    }
    if (mdec->phase == 2) {
        /*  マウスの 2バイト目を待っている段階  */
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return ( 0 );
    }
    if (mdec->phase == 3) {
        /*  マウスの 3バイト目を待っている段階  */
        mdec->buf[2] = dat;
        mdec->phase = 1;
        mdec->btn = (mdec->buf[0] & 0x07);
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];
        if ((mdec->buf[0] & 0x10) != 0) {
            mdec->x |= 0xffffff00;
        }
        if ((mdec->buf[0] & 0x20) != 0) {
            mdec->y |= 0xffffff00;
        }
        mdec->y = -(mdec->y);   /*  マウスでは縦方向の符号が画面と逆。  */
        return ( 1 );
    }
    /*  ここにくることはないはず。  */
    return ( -1 );
}
