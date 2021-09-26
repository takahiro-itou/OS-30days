/*  マウス関係  */

#include "BootPack.h"

struct FIFO8 mousefifo;

/*  PS/2  マウスからの割り込み  */
void inthandler2c(int *esp)
{
    unsigned char data;

    io_out8(PIC1_OCW2, 0x64);   /*  IRQ-12 受付完了を PIC に通知。  */
    io_out8(PIC0_OCW2, 0x62);   /*  IRQ-02 受付完了を PIC に通知。  */
    data = io_in8(PORT_KEYDAT);
    fifo8_put(&mousefifo, data);

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
