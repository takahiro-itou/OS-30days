/*  bootpack  のメイン  */

#include "BootPack.h"
#include "../Common/stdio.h"

#define KEYCMD_LED      0xed

static const char keytable0[0x80] = {
    0  ,  0 , '1', '2',     '3', '4', '5', '6',
    '7', '8', '9', '0',     '-', '^',0x08,  0 ,
    'Q', 'W', 'E', 'R',     'T', 'Y', 'U', 'I',
    'O', 'P', '@', '[',    0x0a,  0 , 'A', 'S',
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
    '\'','(', ')', '~',     '=', '~',0x08,  0 ,
    'Q', 'W', 'E', 'R',     'T', 'Y', 'U', 'I',
    'O', 'P', '`', '{',    0x0a,  0 , 'A', 'S',
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

void keywin_off(struct SHEET *key_win);
void keywin_on(struct SHEET *key_win);


void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *)(ADR_BOOTINFO);
    struct KERNELWORK kw;
    struct MAIN_VARS kmv;

    char s[40], keyseq[32];
    struct FIFO32 fifo, keycmd;
    int fifobuf[128], keycmd_buf[32], *cons_fifo[MAX_CONSOLE];
    int i;
    unsigned int memtotal;
    struct MOUSE_DEC mdec;
    struct MEMMAN*memman = (struct MEMMAN *)(MEMMAN_ADDR);
    unsigned char *buf_back, buf_mouse[256];
    struct SHEET *sht_back;
    struct TASK *task_a, *task_cons[MAX_CONSOLE];

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
    for (i = 0; i < MAX_CONSOLE; ++ i) {
        kmv.sht_cons[i] = sheet_alloc(kmv.shtctl);
        kmv.buf_cons[i] = (unsigned char *) memman_alloc_4k(
                memman, CONSOLE_WIN_SIZE_X * CONSOLE_WIN_SIZE_Y);
        sheet_setbuf(kmv.sht_cons[i], kmv.buf_cons[i],
                     CONSOLE_WIN_SIZE_X, CONSOLE_WIN_SIZE_Y, -1);
        make_window8(kmv.buf_cons[i],
                     CONSOLE_WIN_SIZE_X, CONSOLE_WIN_SIZE_Y,
                     "console", 0);
        make_textbox8(kmv.sht_cons[i], CURSOR_LEFT, CURSOR_TOP,
                      (CONSOLE_COLS * CURSOR_WIDTH),
                      (CONSOLE_ROWS * CURSOR_HEIGHT),
                      COL8_000000);

        task_cons[i] = task_alloc();
        kmv.task_cons[i] = task_cons[i];
        task_cons[i]->tss.esp  = memman_alloc_4k(memman, 64 * 1024)
                + 64 * 1024 - 12;
        task_cons[i]->tss.eip  = (int) &console_task;
        task_cons[i]->tss.es   = 1 * 8;
        task_cons[i]->tss.cs   = 2 * 8;
        task_cons[i]->tss.ss   = 1 * 8;
        task_cons[i]->tss.ds   = 1 * 8;
        task_cons[i]->tss.fs   = 1 * 8;
        task_cons[i]->tss.gs   = 1 * 8;
        *((int *) (task_cons[i]->tss.esp + 4)) = (int) kmv.sht_cons[i];
        *((int *) (task_cons[i]->tss.esp + 8)) = memtotal;
        task_run(task_cons[i], 2, 2);

        kmv.sht_cons[i]->task = kmv.task_cons[i];
        kmv.sht_cons[i]->flags |= 0x20;     /*  カーソルあり。  */
        cons_fifo[i] = (int *) memman_alloc_4k(memman, 128 * 4);
        fifo32_init(&task_cons[i]->fifo, 128, cons_fifo[i], task_cons[i]);
    }

    /*  sht_mouse   */
    kmv.sht_mouse = sheet_alloc(kmv.shtctl);
    sheet_setbuf(kmv.sht_mouse, buf_mouse, 16, 16, 99);
    init_mouse_cursor8(buf_mouse, 99);
    kw.mx = (binfo->scrnx - 16) / 2;
    kw.my = (binfo->scrny - 28 - 16) / 2;
    kw.mmx = -1;
    kw.mmy = -1;
    kw.mmx2 = 0;

    sheet_slide(    sht_back,      0,  0);
    sheet_slide(kmv.sht_cons[1],  56,  6);
    sheet_slide(kmv.sht_cons[0],   8,  2);
    sheet_slide(kmv.sht_mouse, kw.mx, kw.my);

    sheet_updown(    sht_back,     0);
    sheet_updown(kmv.sht_cons[1],  1);
    sheet_updown(kmv.sht_cons[0],  2);
    sheet_updown(kmv.sht_mouse,    3);
    kw.key_win = kmv.sht_cons[0];
    keywin_on(kw.key_win);

    /*  最初にキーボード状態との食い違いがないように、設定しておく  */
    fifo32_put(&keycmd, KEYCMD_LED);
    fifo32_put(&keycmd, kw.key_leds);

    for (i = 0; i < sizeof(keyseq); ++ i) {
        keyseq[i] = ' ';
    }
    keyseq[sizeof(keyseq) - 1] = 0;

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
                keywin_on(kw.key_win);
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
            }
        }
    }
}

void process_key_data(
        struct KERNELWORK *pkw, int code, struct MAIN_VARS *vars)
{
    struct KERNELWORK kw = (* pkw);
    int i = code + 256;
    struct SHTCTL *shtctl = vars->shtctl;
    struct FIFO32 *pkeycmd = vars->keycmd;

    struct TASK *task;
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
        /*  通常文字、バックスペース、Enter.    */
        fifo32_put(&kw.key_win->task->fifo, s[0] + 256);
    }
    if (i == 256 + 0x0f) {      /*  Tab */
        keywin_off(kw.key_win);
        j = kw.key_win->height - 1;
        if (j == 0) {
            j = shtctl->top - 1;
        }
        kw.key_win = shtctl->sheets[j];
        keywin_on(kw.key_win);
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
    if (i == 256 + 0x3b && kw.key_shift != 0)
    {
        /*  Shift + F1  */
        task = kw.key_win->task;
        if (task != 0 && task->tss.ss0 != 0) {
            cons_putstr0(task->cons, "\nBreak(key) :\n");
            io_cli();   /*  レジスタ変更中にタスクが変わると困る。  */
            task->tss.eax = (int) &(task->tss.esp0);
            task->tss.eip = (int) asm_end_app;
            io_sti();
        }
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
    struct SHTCTL *shtctl = vars->shtctl;
    struct SHEET *key_win = pkw->key_win;

    struct TASK *task;

    sheet_updown(sht, shtctl->top - 1);
    if (sht != key_win) {
        keywin_off(key_win);
        key_win = sht;
        keywin_on(key_win);
    }

    if (3 <= x && x < sht->bxsize && 3 <= y && y < 21)
    {
        /*  ウィンドウ移動モードへ  */
        pkw->mmx = pkw->mx;
        pkw->mmy = pkw->my;
        pkw->mmx2 = sht->vx0;
    }

    if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 3 <= y && y < 19)
    {
        /*  「×」ボタンクリック。  */
        if ((sht->flags & 0x10) != 0) {
            /*  アプリが作ったウィンドウ。  */
            task = sht->task;
            cons_putstr0(task->cons, "\nBreak(mouse) :\n");
            io_cli();
            task->tss.eax = (int) &(task->tss.esp0);
            task->tss.eip = (int) asm_end_app;
            io_sti();
        }
    }

    pkw->key_win = key_win;
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
                        sheet_leftbutton_down(&kw, sht, x, y, vars);
                        break;
                    }
                }
            }
        } else {
            /*  ウィンドウ移動モードの場合  */
            x = kw.mx - kw.mmx;
            y = kw.my - kw.mmy;
            sheet_slide(sht, (kw.mmx2 + x +2) & ~3, sht->vy0 + y);
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

void keywin_off(struct SHEET *key_win)
{
    change_wtitle8(key_win, 0);
    if ((key_win->flags & 0x20) != 0) {
        /*  コンソールのカーソル OFF    */
        fifo32_put(&key_win->task->fifo, 3);
    }

    return;
}

void keywin_on(struct SHEET *key_win)
{
    change_wtitle8(key_win, 1);
    if ((key_win->flags & 0x20) != 0) {
        /*  コンソールのカーソル ON     */
        fifo32_put(&key_win->task->fifo, 2);
    }

    return;
}
