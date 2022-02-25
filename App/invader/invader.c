
#include "apilib.h"
#include "string.h"

static unsigned char charset[16 * 8] = {

    /*  invader(0)  */
    0x00, 0x00, 0x00, 0x43,  0x5f, 0x5f, 0x5f, 0x7f,
    0x1f, 0x1f, 0x1f, 0x1f,  0x00, 0x20, 0x3f, 0x00,

    /*  invader(1)  */
    0x00, 0x0f, 0x7f, 0xff,  0xcf, 0xcf, 0xcf, 0xff,
    0xff, 0xe0, 0xff, 0xff,  0xc0, 0xc0, 0xc0, 0x00,

    /*  invader(2)  */
    0x00, 0xf0, 0xfe, 0xff,  0xf3, 0xf3, 0xf3, 0xff,
    0xff, 0x07, 0xff, 0xff,  0x03, 0x03, 0x03, 0x00,

    /*  invader(3)  */
    0x00, 0x00, 0x00, 0xc2,  0xfa, 0xfa, 0xfa, 0xfe,
    0xf8, 0xf8, 0xf8, 0xf8,  0x00, 0x04, 0xfc, 0x00,

    /*  fighter(0)  */
    0x00, 0x00, 0x01, 0x01,  0x01, 0x01, 0x01, 0x01,
    0x01, 0x43, 0x47, 0x4f,  0x5f, 0x7f, 0x7f, 0x00,

    /*  fighter(1)  */
    0x18, 0x7e, 0xff, 0xc3,  0xc3, 0xc3, 0xc3, 0xff,
    0xff, 0xff, 0xe7, 0xe7,  0xe7, 0xe7, 0xff, 0x00,

    /*  fighter(2)  */
    0x00, 0x00, 0x80, 0x80,  0x80, 0x80, 0x80, 0x80,
    0x80, 0xc2, 0xe2, 0xf2,  0xfa, 0xfe, 0xfe, 0x00,

    /*  lazer   */
    0x00, 0x18, 0x18, 0x18,  0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18,  0x18, 0x18, 0x18, 0x00
};

void putstr(int win, char *winbuf, int x, int y, int col, unsigned char *s)
{
    int c, x0, i;
    char *p, *q, t[2];
    x = x * 8 + 8;
    y = y * 16 + 29;
    x0 = x;

    i = strlen(s);
    api_boxfilwin(win + 1, x, y, x + i * 8 - 1, y + 15, 0);
    q = winbuf + y * 336;
    t[1] = 0;

    for (;;) {
        c = *s;
        if (c == 0) {
            break;
        }
        if (c != ' ') {
            if ('a' <= c && c <= 'h') {
                p = charset + 16 * (c - 'a');
                q += x;
                for (i = 0; i < 16; ++ i) {
                    if ((p[i] & 0x80) != 0) { q[0] = col; }
                    if ((p[i] & 0x40) != 0) { q[1] = col; }
                    if ((p[i] & 0x20) != 0) { q[2] = col; }
                    if ((p[i] & 0x10) != 0) { q[3] = col; }
                    if ((p[i] & 0x08) != 0) { q[4] = col; }
                    if ((p[i] & 0x04) != 0) { q[5] = col; }
                    if ((p[i] & 0x02) != 0) { q[6] = col; }
                    if ((p[i] & 0x01) != 0) { q[7] = col; }
                    q += 336;
                }
                q -= (336 * 16 + x);
            } else {
                t[0] = *s;
                api_putstrwin(win + 1, x, y, col, 1, t);
            }
        }
        ++ s;
        x += 8;
    }
    api_refreshwin(win, x0, y, x, y + 16);
    return;
}

void wait(int i, int timer, char *keyflag)
{
    int j;
    if (i > 0) {
        /*  一定時間待つ。  */
        api_settimer(timer, i);
        i = 128;
    } else {
        i = 0x0a;   /*  Enter.  */
    }

    for (;;) {
        j = api_getkey(1);
        if (i == j) {
            break;
        }
        if (j == '4') {
            keyflag[0] = 1;     /*  left.   */
        }
        if (j == '6') {
            keyflag[1] = 1;     /*  right.  */
        }
        if (j == ' ') {
            keyflag[2] = 1;     /*  space.  */
        }
    }
    return;
}

void HariMain(void)
{
    int win, timer, i, j, fx, laserwait, lx = 0, ly;
    int ix, iy, movewait0, movewait, idir;
    int invline, score, high, point;
    char winbuf[336 * 261], invstr[32 * 6], s[12], keyflag[4], *p;
    static char invstr0[32] = " abcd abcd abcd abcd abcd ";

    win = api_openwin(winbuf, 336, 261, -1, "invader");
    api_boxfilwin(win, 6, 27, 329, 254, 0);
    timer = api_alloctimer();
    api_inittimer(timer, 128);

    high = 0;
    putstr(win, winbuf, 22, 0, 7, "HIGH:00000000");

restart:
    score = 0;
    point = 1;
    putstr(win, winbuf,  4, 0, 7, "SCORE:00000000");
    movewait0 = 20;
    fx = 18;
    putstr(win, winbuf, fx, 13, 6, "efg");
    wait(100, timer, keyflag);

next_group:
    wait(100, timer, keyflag);
    ix = 7;
    iy = 1;
    invline = 6;
    for (i = 0; i < 6; ++ i) {
        for (j = 0; j < 27; ++ j) {
            invstr[i * 32 + j] = invstr0[j];
        }

        putstr(win, winbuf, ix, iy + i, 2, invstr + i * 32);
    }
    keyflag[0] = 0;
    keyflag[1] = 0;
    keyflag[2] = 0;

    ly = 0;     /*  非表示  */
    laserwait = 0;
    movewait = movewait0;
    idir = +1;
    wait(100, timer, keyflag);

    wait(0, timer, keyflag);
    api_end();
}
