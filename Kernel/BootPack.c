/*  bootpack  のメイン  */

#include "BootPack.h"
#include "../Common/stdio.h"
#include "../Common/string.h"

#define KEYCMD_LED      0xed

void console_task(struct SHEET *sheet, unsigned int memtotal);

void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *)(ADR_BOOTINFO);
    struct SHTCTL *shtctl;
    char s[40], keyseq[32];
    struct FIFO32 fifo, keycmd;
    int fifobuf[128], keycmd_buf[32];
    int mx, my, i, cursor_x, cursor_c;
    unsigned int memtotal;
    struct MOUSE_DEC mdec;
    struct MEMMAN*memman = (struct MEMMAN *)(MEMMAN_ADDR);
    unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;
    struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
    struct TASK *task_a, *task_cons;
    struct TIMER *timer;

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
    int key_to = 0, key_shift = 0;
    int key_leds = (binfo->leds >> 4) & 7;
    int keycmd_wait = -1;

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
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

    task_a = task_init(memman);
    fifo.task = task_a;
    task_run(task_a, 1, 2);

    /*  sht_back    */
    sht_back  = sheet_alloc(shtctl);
    buf_back  = (unsigned char *)memman_alloc_4k(
            memman, binfo->scrnx * binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
    init_screen8(buf_back, binfo->scrnx, binfo->scrny);

    /*  sht_cons    */
    sht_cons = sheet_alloc(shtctl);
    buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
    sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);
    make_window8(buf_cons, 256, 165, "console", 0);
    make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
    task_cons = task_alloc();
    task_cons->tss.esp  = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
    task_cons->tss.eip  = (int) &console_task;
    task_cons->tss.es   = 1 * 8;
    task_cons->tss.cs   = 2 * 8;
    task_cons->tss.ss   = 1 * 8;
    task_cons->tss.ds   = 1 * 8;
    task_cons->tss.fs   = 1 * 8;
    task_cons->tss.gs   = 1 * 8;
    *((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
    *((int *) (task_cons->tss.esp + 8)) = memtotal;
    task_run(task_cons, 2, 2);

    /*  sht_win     */
    sht_win   = sheet_alloc(shtctl);
    buf_win   = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
    sheet_setbuf(sht_win, buf_win, 144, 52, -1);    /*  透明色なし  */
    make_window8(buf_win, 144, 52, "task_a", 1);
    make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
    cursor_x = 8;
    cursor_c = COL8_FFFFFF;

    timer = timer_alloc();
    timer_init(timer, &fifo, 1);
    timer_settime(timer, 50);

    /*  sht_mouse   */
    sht_mouse = sheet_alloc(shtctl);
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
    init_mouse_cursor8(buf_mouse, 99);
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;

    sheet_slide(sht_back,   0,  0);
    sheet_slide(sht_cons,  32, 44);
    sheet_slide(sht_win,   64, 96);
    sheet_slide(sht_mouse, mx, my);

    sheet_updown(sht_back,  0);
    sheet_updown(sht_cons,  1);
    sheet_updown(sht_win,   2);
    sheet_updown(sht_mouse, 3);

    /*  最初にキーボード状態との食い違いがないように、設定しておく  */
    fifo32_put(&keycmd, KEYCMD_LED);
    fifo32_put(&keycmd, key_leds);

    for (i = 0; i < sizeof(keyseq); ++ i) {
        keyseq[i] = ' ';
    }
    keyseq[sizeof(keyseq) - 1] = 0;

    for (;;) {
        if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
            /*  キーボードコントローラに送るデータがあれば、送る。  */
            keycmd_wait = fifo32_get(&keycmd);
            wait_KBC_sendready();
            io_out8(PORT_KEYDAT, keycmd_wait);
        }
        io_cli();
        if (fifo32_status(&fifo) == 0) {
            task_sleep(task_a);
            io_sti();
        } else {
            i = fifo32_get(&fifo);
            io_sti();
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
                if (i < 0x80 + 256) {
                    /*  キーコードを文字コードに変換。  */
                    if (key_shift == 0) {
                        s[0] = keytable0[i - 256];
                    } else {
                        s[0] = keytable1[i - 256];
                    }
                } else {
                    s[0] = 0;
                }
                if ('A' <= s[0] && s[0] <= 'Z') {
                    /*  入力文字がアルファベット。  */
                    if ( ((key_leds & 4) == 0 && key_shift == 0) ||
                            ((key_leds & 4) != 0 && key_shift != 0) )
                    {
                        s[0] += 0x20;
                    }
                }
                if (s[0] != 0) {
                    /*  通常文字。  */
                    if (key_to == 0) {
                        if (cursor_x < 128) {
                            s[1] = 0;
                            putfonts8_asc_sht(sht_win, cursor_x, 28,
                                              COL8_000000, COL8_FFFFFF, s, 1);
                            cursor_x += 8;
                        }
                    } else {
                        fifo32_put(&task_cons->fifo, s[0] + 256);
                    }
                }
                if (i == 256 + 0x0e) {
                    /*  バックスペース  */
                    if (key_to == 0) {
                        if (cursor_x > 8) {
                            putfonts8_asc_sht(sht_win, cursor_x, 28,
                                              COL8_000000, COL8_FFFFFF, " ", 1);
                            cursor_x -= 8;
                        }
                    } else {
                        fifo32_put(&task_cons->fifo, 8 + 256);
                    }
                }
                if (i == 256 + 0x1c) {      /*  Enter   */
                    if (key_to != 0) {
                        fifo32_put(&task_cons->fifo, 10 + 256);
                    }
                }
                if (i == 256 + 0x0f) {      /*  Tab */
                    if (key_to == 0) {
                        key_to = 1;
                        make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  0);
                        make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
                        cursor_c = -1;      /*  カーソルを消す  */
                        boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF,
                                 cursor_x, 28, cursor_x + 7, 43);
                        /*  コンソールのカーソルON  */
                        fifo32_put(&task_cons->fifo, 2);
                    } else {
                        key_to = 0;
                        make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
                        make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
                        cursor_c = COL8_000000;     /*  カーソルを出す  */
                        /*  コンソールのカーソル OFF    */
                        fifo32_put(&task_cons->fifo, 3);
                    }
                    sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
                    sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
                }
                /*  シフトキー  */
                if (i == 256 + 0x2a) {
                    key_shift |= 1;
                }
                if (i == 256 + 0x36) {
                    key_shift |= 2;
                }
                if (i == 256 + 0xaa) {
                    key_shift &= ~1;
                }
                if (i == 256 + 0xb6) {
                    key_shift &= ~2;
                }
                if (i == 256 + 0x3a) {  /*  CapsLock    */
                    key_leds ^= 4;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if (i == 256 + 0x45) {  /*  NumLock     */
                    key_leds ^= 2;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if (i == 256 + 0x46) {  /*  ScrollLock  */
                    key_leds ^= 1;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if (i == 256 + 0xfa) {
                    /*  キーボードがデータを無事に受け取った。  */
                    keycmd_wait = -1;
                }
                if (i == 256 + 0xfe) {
                    /*  キーボードがデータを無事に受け取れなかった  */
                    wait_KBC_sendready();
                    io_out8(PORT_KEYDAT, keycmd_wait);
                }
                /*  カーソルの再表示。  */
                if (cursor_c >= 0) {
                    boxfill8(sht_win->buf, sht_win->bxsize, cursor_c,
                             cursor_x, 28, cursor_x + 7, 43);
                }
                sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
            } else if (512 <= i && i <= 767) {
                /*  マウスデータ。      */
                if (mouse_decode(&mdec, i - 512) != 0) {
                    /*  マウスカーソルの移動。  */
                    mx += mdec.x;
                    my += mdec.y;
                    if (mx < 0) {
                        mx = 0;
                    }
                    if (my < 0) {
                        my = 0;
                    }
                    if (mx > binfo->scrnx - 1) {
                        mx = binfo->scrnx - 1;
                    }
                    if (my > binfo->scrny - 1) {
                        my = binfo->scrny - 1;
                    }

                    sheet_slide(sht_mouse, mx, my);
                    if ((mdec.btn & 0x01) != 0) {
                        /*  左ボタンを押していたら、sht_win を動かす。  */
                        sheet_slide(sht_win, mx - 80, my - 8);
                    }
                }
            } else if (i <= 1) {    /*  カーソル用タイマ。  */
                if (i != 0) {
                    timer_init(timer, &fifo, 0);
                    if (cursor_c >= 0) {
                        cursor_c = COL8_000000;
                    }
                } else {
                    timer_init(timer, &fifo, 1);
                    if (cursor_c >= 0) {
                        cursor_c = COL8_FFFFFF;
                    }
                }
                timer_settime(timer, 50);
                if (cursor_c >= 0) {
                    boxfill8(sht_win->buf, sht_win->bxsize, cursor_c,
                             cursor_x, 28, cursor_x + 7, 43);
                    sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
                }
            }
        }
    }
}

void make_wtitle8(unsigned char *buf, int xsize, char *title, char act)
{
    static const char closebtn[14][16] = {
        "OOOOOOOOOOOOOOO@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQQQ@@QQQQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "O$$$$$$$$$$$$$$@",
        "@@@@@@@@@@@@@@@@"
    };

    int x, y;
    char c, tc, tbc;

    if (act != 0) {
        tc  = COL8_FFFFFF;
        tbc = COL8_000084;
    } else {
        tc  = COL8_C6C6C6;
        tbc = COL8_848484;
    }
    boxfill8(buf, xsize, tbc, 3, 3, xsize - 4, 20);
    putfonts8_asc(buf, xsize, 24, 4, tc, title);

    for (y = 0; y < 14; ++ y) {
        for (x = 0; x < 16; ++ x) {
            c = closebtn[y][x];
            if (c == '@') {
                c = COL8_000000;
            } else if (c == '$') {
                c = COL8_848484;
            } else if (c == 'Q') {
                c = COL8_C6C6C6;
            } else {
                c = COL8_FFFFFF;
            }
            buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
        }
    }
    return;
}

void make_window8(unsigned char *buf, int xsize, int ysize,
                  char *title, char act)
{

    boxfill8(buf, xsize, COL8_C6C6C6, 0, 0,         xsize - 1, 0        );
    boxfill8(buf, xsize, COL8_FFFFFF, 1, 1,         xsize - 2, 1        );
    boxfill8(buf, xsize, COL8_C6C6C6, 0, 0,         0,         ysize - 1);
    boxfill8(buf, xsize, COL8_FFFFFF, 1, 1,         1,         ysize - 2);
    boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
    boxfill8(buf, xsize, COL8_C6C6C6, 2, 2,         xsize - 3, ysize - 3);
    boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
    make_wtitle8(buf, xsize, title, act);
    return;
}

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
    int x1 = x0 + sx, y1 = y0 + sy;
    boxfill8(sht->buf, sht->bxsize, COL8_848484, x0-2, y0-3, x1+1, y0-3);
    boxfill8(sht->buf, sht->bxsize, COL8_848484, x0-3, y0-3, x0-3, y1+1);
    boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0-3, y1+2, x1+1, y1+2);
    boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1+2, y0-3, x1+2, y1+2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0-1, y0-2, x1+0, y0-2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0-2, y0-2, x0-2, y1+0);
    boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0-2, y1+1, x1+0, y1+1);
    boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1+1, y0-2, x1+1, y1+1);
    boxfill8(sht->buf, sht->bxsize, c,           x0-1, y0-1, x1+0, y1+0);
    return;
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b,
                       const char *s, int l)
{
    boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
    putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
    sheet_refresh(sht, x, y, x + l * 8, y + 16);
    return;
}

int cons_newline(int cursor_y, struct SHEET *sheet)
{
    int x, y;

    if (cursor_y < 28 + 112) {
        cursor_y += 16;     /*  次の行へ。  */
    } else {
        /*  スクロール  */
        for (y = 28; y < 28 + 112; ++ y) {
            for (x = 8; x < 8 + 240; ++ x){
                sheet->buf[x + y*sheet->bxsize] =
                        sheet->buf[x + (y + 16) * sheet->bxsize];
            }
        }
        boxfill8(sheet->buf, sheet->bxsize, COL8_000000,
                 8, 28 + 112, 8 + 240 - 1, 28 + 128 - 1);
        sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
    }
    return cursor_y;

}

void file_readfat(int *fat, unsigned char *img)
{
    int i, j = 0;
    for (i = 0; i < 2880; i += 2) {
        fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
        fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
        j += 3;
    }
    return;
}

void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
    int i;
    for (;;) {
        if (size <= 512) {
            for (i = 0; i < size; ++ i) {
                buf[i] = img[clustno * 512 + i];
            }
            break;
        }
        for (i = 0; i < 512; ++ i) {
            buf[i] = img[clustno * 512 + i];
        }
        size -= 512;
        buf  += 512;
        clustno = fat[clustno];
    }
    return;
}

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
    struct TIMER *timer;
    struct TASK *task = task_now();
    int i, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_c = -1;
    char s[30], cmdline[30], *p;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    int x, y;
    struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
    int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);

    fifo32_init(&task->fifo, sizeof(fifobuf), fifobuf, task);

    timer = timer_alloc();
    timer_init(timer, &task->fifo, 1);
    timer_settime(timer, 50);
    file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));

    /*  プロンプト表示  */
    putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

    for (;;) {
        io_cli();
        if (fifo32_status(&task->fifo) == 0) {
            task_sleep(task);
            io_sti();
        } else {
            i = fifo32_get(&task->fifo);
            io_sti();
            if (i <= 1) {   /*  カーソル用タイマ。  */
                if (i != 0) {
                    timer_init(timer, &task->fifo, 0);
                    if (cursor_c >= 0) {
                        cursor_c = COL8_FFFFFF;
                    }
                } else {
                    timer_init(timer, &task->fifo, 1);
                    if (cursor_c >= 0) {
                        cursor_c = COL8_000000;
                    }
                }
                timer_settime(timer, 50);
            }
            if (i == 2) {       /*  カーソル ON */
                cursor_c = COL8_FFFFFF;
            }
            if (i == 3) {       /*  カーソル OFF    */
                boxfill8(sheet->buf, sheet->bxsize, COL8_000000,
                         cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
                cursor_c = -1;
            }
            if (256 <= i && i <= 511) {
                /*  キーボードデータ。  */
                if (i == 8 + 256) {
                    /*  バックスペース  */
                    if (cursor_x > 16) {
                        putfonts8_asc_sht(sheet, cursor_x, cursor_y,
                                          COL8_FFFFFF, COL8_000000, " ", 1);
                        cursor_x -= 8;
                    }
                } else if (i == 10 + 256) {
                    /*  Enter.  */
                    /*  カーソルをスペースで消す。  */
                    putfonts8_asc_sht(sheet, cursor_x, cursor_y,
                                      COL8_FFFFFF, COL8_000000, " ", 1);
                    cmdline[cursor_x / 8 - 2] = 0;
                    cursor_y = cons_newline(cursor_y, sheet);
                    /*  コマンド実行。  */
                    if (strcmp(cmdline, "mem") == 0) {
                        /*  mem コマンド。  */
                        snprintf(s, sizeof(s), "total   %dMB",
                                 memtotal / (1024 * 1024));
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF,
                                          COL8_000000, s, 30);
                        cursor_y = cons_newline(cursor_y, sheet);
                        snprintf(s, sizeof(s), "free %dKB",
                                 memman_total(memman) / 1024);
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF,
                                          COL8_000000, s, 30);
                        cursor_y = cons_newline(cursor_y, sheet);
                        cursor_y = cons_newline(cursor_y, sheet);
                    } else if (strcmp(cmdline, "cls") == 0) {
                        boxfill8(sheet->buf, sheet->bxsize, COL8_000000,
                                 8, 28, 8 + 240 - 1, 28 + 128 - 1);
                        sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
                        cursor_y = 28;
                    } else if (strcmp(cmdline, "dir") == 0) {
                        for (x = 0; x < 224; ++ x) {
                            if (finfo[x].name[0] == 0x00) {
                                break;
                            }
                            if (finfo[x].name[0] == 0xe5) {
                                continue;
                            }
                            if((finfo[x].type & 0x18) == 0) {
                                snprintf(s, sizeof(s), "filename.ext   %7d",
                                         finfo[x].size);
                                for (y = 0; y < 8; ++y) {
                                    s[y] = finfo[x].name[y];
                                }
                                s[ 9] = finfo[x].ext[0];
                                s[10] = finfo[x].ext[1];
                                s[11] = finfo[x].ext[2];
                                putfonts8_asc_sht(sheet, 8, cursor_y,
                                                  COL8_FFFFFF,
                                                  COL8_000000,
                                                  s, 30);
                                cursor_y = cons_newline(cursor_y, sheet);
                            }
                        }
                        cursor_y = cons_newline(cursor_y, sheet);
                    } else if (strncmp(cmdline, "type ", 5) == 0) {
                        /*  ファイル名を準備する。  */
                        for (y = 0; y < 11; ++ y) {
                            s[y] = ' ';
                        }
                        s[y] = 0;
                        y = 0;
                        for (x = 5; y < 11 && cmdline[x] != 0; ++ x) {
                            if (cmdline[x] == '.' && y <= 8) {
                                y = 8;
                            } else {
                                s[y] = cmdline[x];
                                if ('a' <= s[y] && s[y] <= 'z') {
                                    s[y] -= 0x20;
                                }
                                ++ y;
                            }
                        }
                        /*  ファイルを探す  */
                        for (x = 0; x < 224; ) {
                            if (finfo[x].name[0] == 0x00) {
                                break;
                            }
                            if ((finfo[x].type & 0x18) == 0) {
                                for (y = 0; y < 8; ++ y) {
                                    if (finfo[x].name[y] != s[y]) {
                                        goto type_next_file;
                                    }
                                }
                                for (y = 0; y < 3; ++ y) {
                                    if (finfo[x].ext[y] != s[y + 8]) {
                                        goto type_next_file;
                                    }
                                }
                                break;
                            }
type_next_file:
                            ++ x;
                        }

                        if (x < 224 && finfo[x].name[0] != 0x00) {
                            /*  ファイルが見つかった場合。  */
                            p = (char *) memman_alloc_4k(memman, finfo[x].size);
                            file_loadfile(finfo[x].clustno, finfo[x].size,
                                          p, fat,
                                          (char *) (ADR_DISKIMG + 0x003e00));
                            cursor_x = 8;
                            for (y = 0; y < finfo[x].size; ++ y) {
                                s[0] = p[y];
                                s[1] = 0;
                                if (s[0] == 0x09) {
                                    for(;;) {
                                        putfonts8_asc_sht(sheet, cursor_x,
                                                          cursor_y,
                                                          COL8_FFFFFF,
                                                          COL8_000000,
                                                          " ", 1);
                                        cursor_x += 8;
                                        if (cursor_x == 8 + 240) {
                                            cursor_x = 8;
                                            cursor_y = cons_newline(cursor_y,
                                                                    sheet);
                                        }
                                        if (((cursor_x - 8) & 0x1f) == 0) {
                                            break;
                                        }
                                    }
                                } else if (s[0]== 0x0a) {
                                    cursor_x = 8;
                                    cursor_y = cons_newline(cursor_y, sheet);
                                } else if (s[0] == 0x0d) {
                                } else {
                                    putfonts8_asc_sht(sheet, cursor_x, cursor_y,
                                                  COL8_FFFFFF, COL8_000000,
                                                  s, 1);
                                    cursor_x += 8;
                                    if (cursor_x == 8 + 240) {
                                        cursor_x = 8;
                                        cursor_y = cons_newline(cursor_y,
                                                                sheet);
                                    }
                                }
                            }
                            memman_free_4k(memman, (int)p, finfo[x].size);
                        } else {
                            /*  ファイルが見つからなかった場合  */
                            putfonts8_asc_sht(sheet, 8, cursor_y,
                                              COL8_FFFFFF, COL8_000000,
                                              "File not found.", 15);
                            cursor_y = cons_newline(cursor_y, sheet);
                        }
                        cursor_y = cons_newline(cursor_y, sheet);
                    } else if (cmdline[0] != 0) {
                        /*  コマンドではなく、さらに空行でもない。  */
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF,
                                          COL8_000000, "Bad command.", 12);
                        cursor_y = cons_newline(cursor_y, sheet);
                        cursor_y = cons_newline(cursor_y, sheet);
                    }
                    /*  プロンプト表示  */
                    putfonts8_asc_sht(sheet, 8, cursor_y,
                                      COL8_FFFFFF, COL8_000000, ">", 1);
                    cursor_x = 16;
                } else {
                    if (cursor_x < 240) {
                        s[0] = i - 256;
                        s[1] = 0;
                        cmdline[cursor_x / 8 - 2] = i - 256;
                        putfonts8_asc_sht(sheet, cursor_x, cursor_y,
                                          COL8_FFFFFF, COL8_000000, s, 1);
                        cursor_x += 8;
                    }
                }
            }
            /*  カーソル再表示  */
            boxfill8(sheet->buf, sheet->bxsize, cursor_c,
                     cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
            sheet_refresh(sheet, cursor_x, cursor_y,
                          cursor_x + 8, cursor_y + 16);
        }
    }
}
