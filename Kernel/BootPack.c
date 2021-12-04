/*  bootpack  のメイン  */

#include "BootPack.h"
#include "../Common/stdio.h"

#define KEYCMD_LED      0xed

static const char keytable0[0x80] = {
    0  ,  0 , '1', '2',     '3', '4', '5', '6',
    '7', '8', '9', '0',     '-', '^',  0 ,  0 ,
    'Q', 'W', 'E', 'R',     'T', 'Y', 'U', 'I',
    'O', 'P', '@', '[',      0 ,  0 , 'A', 'S',
    'D', 'F', 'G', 'H',     'J', 'K', 'L', ';',
    ':',  0 ,  0 , ']',     'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',',     '.', '/',  0 , '*',
    0  , ' ',  0 ,  0 ,      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 ,  0 ,      0 ,  0 ,  0 , '7',
    '8', '9', '-', '4',     '5', '6', '+', '1',
    '2', '3', '0', '.',      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 ,  0 ,      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 ,  0 ,      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 ,  0 ,      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 ,0x5c,      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 ,  0,       0 ,0x5c,  0 ,  0
};

static const char keytable1[0x80] = {
    0  ,  0 , '!', '"',     '#', '$', '%', '&',
    '\'','(', ')', '~',     '=', '~',  0 ,  0 ,
    'Q', 'W', 'E', 'R',     'T', 'Y', 'U', 'I',
    'O', 'P', '`', '{',      0 ,  0 , 'A', 'S',
    'D', 'F', 'G', 'H',     'J', 'K', 'L', '+',
    '*',  0 ,  0 , '}',     'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<',     '>', '?',  0 , '*',
    0  , ' ',  0 ,  0 ,      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 ,  0 ,      0 ,  0 ,  0 , '7',
    '8', '9', '-', '4',     '5', '6', '+', '1',
    '2', '3', '0', '.',      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 ,  0 ,      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 ,  0 ,      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 ,  0 ,      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 , '_',      0 ,  0 ,  0 ,  0,
    0  ,  0 ,  0 ,  0,       0 , '|',  0 ,  0
};


void process_key_data(
        struct KERNELWORK *pkw, int code, struct MAIN_VARS *vars);

void process_mouse_data(
        struct KERNELWORK *pkw, struct MOUSE_DEC mdec,
        struct MAIN_VARS *vars);

int keywin_off(
        struct SHEET *key_win, struct SHEET *sht_win, int cur_c, int cur_x);
int keywin_on(struct SHEET *key_win, struct SHEET *sht_win, int cur_c);

void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *)(ADR_BOOTINFO);
    struct KERNELWORK kw;
    struct MAIN_VARS kmv;

    char s[40], keyseq[32];
    struct FIFO32 fifo, keycmd;
    int fifobuf[128], keycmd_buf[32];
    int i;
    unsigned int memtotal;
    struct MOUSE_DEC mdec;
    struct MEMMAN*memman = (struct MEMMAN *)(MEMMAN_ADDR);
    unsigned char *buf_back, buf_mouse[256];
    struct SHEET *sht_back, *sht_win;
    struct TASK *task_a, *task_cons;
    struct TIMER *timer;
    struct CONSOLE *cons;

    kmv.binfo = binfo;
    kmv.keycmd = &keycmd;
    kw.selsht = 0;
    kw.key_win = 0;
    kw.key_shift = 0;
    kw.key_leds = (binfo->leds >> 4) & 7;
    kw.keycmd_wait = -1;

    init_gdtidt();
    init_pic();
    io_sti();

    fifo32_init(&fifo, sizeof(fifobuf), fifobuf, 0);
    init_pit();
    init_keyboard(&fifo, 256);
    enable_mouse(&fifo, 512, &mdec);
    io_out8(PIC0_IMR, 0xf8);    /*  PIT と PIC1 とキーボードを許可  */
    io_out8(PIC1_IMR, 0xef);    /*  マウスを許可            */
    fifo32_init(&keycmd, sizeof(keycmd_buf), keycmd_buf, 0);

    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    init_palette();
    kmv.shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

    task_a = task_init(memman);
    fifo.task = task_a;
    task_run(task_a, 1, 2);
    *((int *) 0x0fe4) = (int) (kmv.shtctl);

    /*  sht_back    */
    sht_back  = sheet_alloc(kmv.shtctl);
    buf_back  = (unsigned char *)memman_alloc_4k(
            memman, binfo->scrnx * binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
    init_screen8(buf_back, binfo->scrnx, binfo->scrny);

    /*  sht_cons    */
    kmv.sht_cons = sheet_alloc(kmv.shtctl);
    kmv.buf_cons = (unsigned char *) memman_alloc_4k(
            memman, CONSOLE_WIN_SIZE_X * CONSOLE_WIN_SIZE_Y);
    sheet_setbuf(kmv.sht_cons, kmv.buf_cons,
                 CONSOLE_WIN_SIZE_X, CONSOLE_WIN_SIZE_Y, -1);
    make_window8(kmv.buf_cons,
                 CONSOLE_WIN_SIZE_X, CONSOLE_WIN_SIZE_Y,
                 "console", 0);
    make_textbox8(kmv.sht_cons, CURSOR_LEFT, CURSOR_TOP,
                  (CONSOLE_COLS * CURSOR_WIDTH),
                  (CONSOLE_ROWS * CURSOR_HEIGHT),
                  COL8_000000);
    task_cons = task_alloc();
    kmv.task_cons = task_cons;
    task_cons->tss.esp  = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
    task_cons->tss.eip  = (int) &console_task;
    task_cons->tss.es   = 1 * 8;
    task_cons->tss.cs   = 2 * 8;
    task_cons->tss.ss   = 1 * 8;
    task_cons->tss.ds   = 1 * 8;
    task_cons->tss.fs   = 1 * 8;
    task_cons->tss.gs   = 1 * 8;
    *((int *) (task_cons->tss.esp + 4)) = (int) kmv.sht_cons;
    *((int *) (task_cons->tss.esp + 8)) = memtotal;
    task_run(task_cons, 2, 2);

    /*  sht_win     */
    kmv.sht_win   = sheet_alloc(kmv.shtctl);
    kmv.buf_win   = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
    sheet_setbuf(kmv.sht_win, kmv.buf_win, 144, 52, -1);    /*  透明色なし  */
    make_window8(kmv.buf_win, 144, 52, "task_a", 1);
    make_textbox8(kmv.sht_win, 8, 28, 128, 16, COL8_FFFFFF);
    kw.cursor_x = 8;
    kw.cursor_c = COL8_FFFFFF;

    timer = timer_alloc();
    timer_init(timer, &fifo, 1);
    timer_settime(timer, 50);

    /*  sht_mouse   */
    kmv.sht_mouse = sheet_alloc(kmv.shtctl);
    sheet_setbuf(kmv.sht_mouse, buf_mouse, 16, 16, 99);
    init_mouse_cursor8(buf_mouse, 99);
    kw.mx = (binfo->scrnx - 16) / 2;
    kw.my = (binfo->scrny - 28 - 16) / 2;
    kw.mmx = -1;
    kw.mmy = -1;

    sheet_slide(    sht_back,   0,  0);
    sheet_slide(kmv.sht_cons,  32, 44);
    sheet_slide(kmv.sht_win,   64, 96);
    sheet_slide(kmv.sht_mouse, kw.mx, kw.my);

    sheet_updown(    sht_back,  0);
    sheet_updown(kmv.sht_cons,  1);
    sheet_updown(kmv.sht_win,   2);
    sheet_updown(kmv.sht_mouse, 3);
    kw.key_win = kmv.sht_win;
    kmv.sht_cons->task = kmv.task_cons;
    kmv.sht_cons->flags |= 0x20;

    /*  最初にキーボード状態との食い違いがないように、設定しておく  */
    fifo32_put(&keycmd, KEYCMD_LED);
    fifo32_put(&keycmd, kw.key_leds);

    for (i = 0; i < sizeof(keyseq); ++ i) {
        keyseq[i] = ' ';
    }
    keyseq[sizeof(keyseq) - 1] = 0;
    sht_win = kmv.sht_win;

    for (;;) {
        if (fifo32_status(&keycmd) > 0 && kw.keycmd_wait < 0) {
            /*  キーボードコントローラに送るデータがあれば、送る。  */
            kw.keycmd_wait = fifo32_get(&keycmd);
            wait_KBC_sendready();
            io_out8(PORT_KEYDAT, kw.keycmd_wait);
        }
        io_cli();

        if (fifo32_status(&fifo) == 0) {
            task_sleep(task_a);
            io_sti();
        } else {
            i = fifo32_get(&fifo);
            io_sti();
            if (kw.key_win->flags == 0) {
                /*  入力ウィンドウが閉じられた  */
                kw.key_win = kmv.shtctl->sheets[kmv.shtctl->top - 1];
                kw.cursor_c = keywin_on(kw.key_win, sht_win, kw.cursor_c);
            }
            if (256 <= i && i <= 511) {
                /*  キーボードデータ。  */
#if 0
                snprintf(s, sizeof(s), "%02X", i - 256);
                for (int j = 17; j >= 3; -- j) {
                    keyseq[j] = keyseq[j - 3];
                }
                keyseq[0] = s[0];
                keyseq[1] = s[1];
                keyseq[2] = ' ';
                keyseq[17] = 0;
                putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF,
                                  COL8_008484, keyseq, 17);
# endif
                process_key_data(&kw, i - 256, &kmv);
            } else if (512 <= i && i <= 767) {
                /*  マウスデータ。      */
                if (mouse_decode(&mdec, i - 512) != 0) {
                    process_mouse_data(&kw, mdec, &kmv);
                }
            } else if (i <= 1) {    /*  カーソル用タイマ。  */
                if (i != 0) {
                    timer_init(timer, &fifo, 0);
                    if (kw.cursor_c >= 0) {
                        kw.cursor_c = COL8_000000;
                    }
                } else {
                    timer_init(timer, &fifo, 1);
                    if (kw.cursor_c >= 0) {
                        kw.cursor_c = COL8_FFFFFF;
                    }
                }
                timer_settime(timer, 50);
                if (kw.cursor_c >= 0) {
                    boxfill8(sht_win->buf, sht_win->bxsize, kw.cursor_c,
                             kw.cursor_x, 28, kw.cursor_x + 7, 43);
                    sheet_refresh(sht_win, kw.cursor_x, 28,
                                  kw.cursor_x + 8, 44);
                }
            }
        }
    }
}

void process_key_data(
        struct KERNELWORK *pkw, int code, struct MAIN_VARS *vars)
{
    struct KERNELWORK kw = (* pkw);
    int i = code + 256;
    struct TASK *task_cons = vars->task_cons;
    struct SHTCTL *shtctl = vars->shtctl;
    unsigned char *buf_win = vars->buf_win;
    unsigned char *buf_cons = vars->buf_cons;
    struct SHEET *sht_win = vars->sht_win;
    struct SHEET *sht_cons = vars->sht_cons;
    struct FIFO32 *pkeycmd = vars->keycmd;
    struct CONSOLE *cons;
    int j;
    char s[40];

    if (i < 0x80 + 256) {
        /*  キーコードを文字コードに変換。  */
        if (kw.key_shift == 0) {
            s[0] = keytable0[i - 256];
        } else {
            s[0] = keytable1[i - 256];
        }
    } else {
        s[0] = 0;
    }

    if ('A' <= s[0] && s[0] <= 'Z') {
        /*  入力文字がアルファベット。  */
        if ( ((kw.key_leds & 4) == 0 && kw.key_shift == 0) ||
                ((kw.key_leds & 4) != 0 && kw.key_shift != 0) )
        {
            s[0] += 0x20;
        }
    }
    if (s[0] != 0) {
        /*  通常文字。  */
        if (kw.key_win == sht_win) {
            if (kw.cursor_x < 128) {
                s[1] = 0;
                putfonts8_asc_sht(sht_win, kw.cursor_x, 28,
                                  COL8_000000, COL8_FFFFFF, s, 1);
                kw.cursor_x += 8;
            }
        } else {
            /*  コンソールへ。  */
            fifo32_put(&kw.key_win->task->fifo, s[0] + 256);
        }
    }
    if (i == 256 + 0x0e) {
        /*  バックスペース  */
        if (kw.key_win == sht_win) {
            if (kw.cursor_x > 8) {
                putfonts8_asc_sht(sht_win, kw.cursor_x, 28,
                                  COL8_000000, COL8_FFFFFF, " ", 1);
                kw.cursor_x -= 8;
            }
        } else {
            /*  コンソールへ。  */
            fifo32_put(&kw.key_win->task->fifo, 8 + 256);
        }
    }
    if (i == 256 + 0x1c) {      /*  Enter   */
        if (kw.key_win != sht_win) {
            /*  コンソールへ。  */
            fifo32_put(&kw.key_win->task->fifo, 10 + 256);
        }
    }
    if (i == 256 + 0x0f) {      /*  Tab */
        kw.cursor_c = keywin_off(kw.key_win, sht_win,
                                 kw.cursor_c, kw.cursor_x);
        j = kw.key_win->height - 1;
        if (j == 0) {
            j = shtctl->top - 1;
        }
        kw.key_win = shtctl->sheets[j];
        kw.cursor_c = keywin_on(kw.key_win, sht_win, kw.cursor_c);
    }

    /*  シフトキー  */
    if (i == 256 + 0x2a) {
        kw.key_shift |= 1;
    }
    if (i == 256 + 0x36) {
        kw.key_shift |= 2;
    }
    if (i == 256 + 0xaa) {
        kw.key_shift &= ~1;
    }
    if (i == 256 + 0xb6) {
        kw.key_shift &= ~2;
    }

    if (i == 256 + 0x3a) {  /*  CapsLock    */
        kw.key_leds ^= 4;
        fifo32_put(pkeycmd, KEYCMD_LED);
        fifo32_put(pkeycmd, kw.key_leds);
    }
    if (i == 256 + 0x45) {  /*  NumLock     */
        kw.key_leds ^= 2;
        fifo32_put(pkeycmd, KEYCMD_LED);
        fifo32_put(pkeycmd, kw.key_leds);
    }
    if (i == 256 + 0x46) {  /*  ScrollLock  */
        kw.key_leds ^= 1;
        fifo32_put(pkeycmd, KEYCMD_LED);
        fifo32_put(pkeycmd, kw.key_leds);
    }
    if (i == 256 + 0x3b && kw.key_shift != 0 && task_cons->tss.ss0 != 0)
    {
        /*  Shift + F1  */
        cons = (struct CONSOLE *) *((int *) 0x0fec);
        cons_putstr0(cons, "\nBreak(key) :\n");
        io_cli();   /*  レジスタ変更中にタスクが変わると困る。  */
        task_cons->tss.eax = (int) &(task_cons->tss.esp0);
        task_cons->tss.eip = (int) asm_end_app;
        io_sti();
    }
    if (i == 256 + 0x57 && shtctl->top > 2) {   /*  F11 */
        sheet_updown(shtctl->sheets[1], shtctl->top - 1);
    }

    if (i == 256 + 0xfa) {
        /*  キーボードがデータを無事に受け取った。  */
        kw.keycmd_wait = -1;
    }
    if (i == 256 + 0xfe) {
        /*  キーボードがデータを無事に受け取れなかった  */
        wait_KBC_sendready();
        io_out8(PORT_KEYDAT, kw.keycmd_wait);
    }
    /*  カーソルの再表示。  */
    if (kw.cursor_c >= 0) {
        boxfill8(sht_win->buf, sht_win->bxsize, kw.cursor_c,
                 kw.cursor_x, 28, kw.cursor_x + 7, 43);
    }
    sheet_refresh(sht_win, kw.cursor_x, 28, kw.cursor_x + 8, 44);

    (* pkw) = kw;
    return;
}

void sheet_leftbutton_down(
        struct KERNELWORK *pkw,
        struct SHEET *sht,
        const int x,
        const int y,
        struct MAIN_VARS *vars)
{
    struct KERNELWORK kw = (* pkw);
    struct TASK *task_cons = vars->task_cons;
    struct SHTCTL *shtctl = vars->shtctl;

    struct CONSOLE *cons;

    sheet_updown(sht, shtctl->top - 1);
    if (3 <= x && x < sht->bxsize && 3 <= y  && y < 21)
    {
        kw.mmx = kw.mx;
        kw.mmy = kw.my;
    }
    if (sht->bxsize - 21 <= x && x < sht->bxsize - 5
            && 3 <= y && y < 19)
    {
        /*  「×」ボタンクリック。  */
        if ((sht->flags & 0x10) != 0) {
            /*  アプリが作ったウィンドウ。  */
            cons = (struct CONSOLE *) *((int *) 0x0fec);
            cons_putstr0(cons, "\nBreak(mouse) :\n");
            io_cli();
            task_cons->tss.eax = (int) &(task_cons->tss.esp0);
            task_cons->tss.eip = (int) asm_end_app;
            io_sti();
        }
    }

    (* pkw) = kw;
    return;
}


void process_mouse_data(
        struct KERNELWORK *pkw, struct MOUSE_DEC mdec,
        struct MAIN_VARS *vars)
{
    struct KERNELWORK kw = (* pkw);
    struct SHEET * sht = pkw->selsht;

    struct BOOTINFO *binfo = vars->binfo;
    struct SHTCTL *shtctl = vars->shtctl;
    struct SHEET *sht_mouse = vars->sht_mouse;

    int j, x, y;


    /*  マウスカーソルの移動。  */
    kw.mx += mdec.x;
    kw.my += mdec.y;
    if (kw.mx < 0) {
        kw.mx = 0;
    }
    if (kw.my < 0) {
        kw.my = 0;
    }
    if (kw.mx > binfo->scrnx - 1) {
        kw.mx = binfo->scrnx - 1;
    }
    if (kw.my > binfo->scrny - 1) {
        kw.my = binfo->scrny - 1;
    }

    sheet_slide(sht_mouse, kw.mx, kw.my);
    if ((mdec.btn & 0x01) != 0) {
        /*  左ボタンを押している。  */
        if (kw.mmx < 0) {
            /*  通常モードの場合。  */
            /*  上の下じきから順番にマウスが指している下じきを探す  */
            for (j = shtctl->top - 1; j > 0; -- j) {
                sht = shtctl->sheets[j];
                x = kw.mx - sht->vx0;
                y = kw.my - sht->vy0;
                if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize)
                {
                    if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
                        sheet_leftbutton_down(pkw, sht, x, y, vars);
                        break;
                    }
                }
            }
        } else {
            /*  ウィンドウ移動モードの場合  */
            x = kw.mx - kw.mmx;
            y = kw.my - kw.mmy;
            sheet_slide(sht, sht->vx0 + x, sht->vy0 + y);
            kw.mmx = kw.mx;
            kw.mmy = kw.my;
        }
    } else {
        /*  左ボタンを押していない  */
        kw.mmx = -1;    /*  通常モードへ。  */
    }

    (* pkw) = kw;
    pkw->selsht = sht;
    return;
}

int keywin_off(
        struct SHEET *key_win, struct SHEET *sht_win, int cur_c, int cur_x)
{
    change_wtitle8(key_win, 0);
    if (key_win == sht_win) {
        cur_c = -1;     /*  カーソルを消す  */
        boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF,
                 cur_x, 28, cur_x + 7, 43);
    } else {
        if ((key_win->flags & 0x20) != 0) {
            /*  コンソールのカーソル OFF    */
            fifo32_put(&key_win->task->fifo, 3);
        }
    }

    return cur_c;
}

int keywin_on(struct SHEET *key_win, struct SHEET *sht_win, int cur_c)
{
    change_wtitle8(key_win, 1);
    if (key_win == sht_win) {
        cur_c = COL8_000000;    /*  カーソルを出す  */
    } else {
        if ((key_win->flags & 0x20) != 0) {
            /*  コンソールのカーソル ON     */
            fifo32_put(&key_win->task->fifo, 2);
        }
    }

    return cur_c;
}
