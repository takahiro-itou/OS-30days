/*  コンソール関係  */

#include "BootPack.h"
#include "../Common/stdio.h"
#include "../Common/string.h"


void console_task(struct SHEET *sheet, unsigned int memtotal)
{
    struct TIMER *timer;
    struct TASK *task = task_now();
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    int i, fifobuf[128];
    int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
    struct CONSOLE cons;

    char s[30], cmdline[30], *p;
    int x, y;
    struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);

    cons.sht = sheet;
    cons.cur_x = CURSOR_LEFT;
    cons.cur_y = CURSOR_TOP;
    cons.cur_c = -1;
    *((int *) 0x0fec) = (int) &cons;

    fifo32_init(&task->fifo, sizeof(fifobuf), fifobuf, task);

    timer = timer_alloc();
    timer_init(timer, &task->fifo, 1);
    timer_settime(timer, 50);
    file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));

    /*  プロンプト表示  */
    cons_putchar(&cons, '>', 1);

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
                    if (cons.cur_c >= 0) {
                        cons.cur_c = COL8_FFFFFF;
                    }
                } else {
                    timer_init(timer, &task->fifo, 1);
                    if (cons.cur_c >= 0) {
                        cons.cur_c = COL8_000000;
                    }
                }
                timer_settime(timer, 50);
            }
            if (i == 2) {       /*  カーソル ON */
                cons.cur_c = COL8_FFFFFF;
            }
            if (i == 3) {       /*  カーソル OFF    */
                boxfill8(sheet->buf, sheet->bxsize, COL8_000000,
                         cons.cur_x, cons.cur_y,
                         cons.cur_x + CURSOR_WIDTH  - 1,
                         cons.cur_y + CURSOR_HEIGHT - 1);
                cons.cur_c = -1;
            }
            if (256 <= i && i <= 511) {
                /*  キーボードデータ。  */
                if (i == 8 + 256) {
                    /*  バックスペース  */
                    if (cons.cur_x > 16) {
                        cons_putchar(&cons, ' ', 0);
                        cons.cur_x -= CURSOR_WIDTH;
                    }
                } else if (i == 10 + 256) {
                    /*  Enter.  */
                    /*  カーソルをスペースで消してから改行する  */
                    cons_putchar(&cons, ' ', 0);
                    cmdline[cons.cur_x / CURSOR_WIDTH - 2] = 0;
                    cons_newline(&cons);
                    cons_runcmd(cmdline, &cons, fat, memtotal);
                    /*  プロンプト表示  */
                    cons_putchar(&cons, '>', 1);
                } else {
                    if (cons.cur_x < CONSOLE_SIZE_X - CURSOR_WIDTH) {
                        cmdline[cons.cur_x / CURSOR_WIDTH - 2] = i - 256;
                        cons_putchar(&cons, i - 256, i);
                    }
                }
            }
            /*  カーソル再表示  */
            if (cons.cur_c >= 0) {
                boxfill8(sheet->buf, sheet->bxsize, cons.cur_c,
                         cons.cur_x, cons.cur_y,
                         cons.cur_x + CURSOR_WIDTH  - 1,
                         cons.cur_y + CURSOR_HEIGHT - 1);
            }
            sheet_refresh(sheet, cons.cur_x, cons.cur_y,
                          cons.cur_x + CURSOR_WIDTH,
                          cons.cur_y + CURSOR_HEIGHT);
        }
    }
}

void cons_putchar(struct CONSOLE *cons, int chr, char move)
{
    char s[2];
    s[0] = chr;
    s[1] = 0;
    if (s[0] == 0x09) {
        for (;;) {
            putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y,
                              COL8_FFFFFF, COL8_000000, " ", 1);
            cons->cur_x += CURSOR_WIDTH;
            if (cons->cur_x == CONSOLE_SIZE_X) {
                cons_newline(cons);
            }
            if (((cons->cur_x - CURSOR_WIDTH) & 0x1f) == 0) {
                break;
            }
        }
    } else if (s[0] == 0x0a) {
        cons_newline(cons);
    } else if (s[0] == 0x0d) {
    } else {
        /*  普通の文字  */
        putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y,
                          COL8_FFFFFF, COL8_000000, s, 1);
        if (move != 0) {
            cons->cur_x += CURSOR_WIDTH;
            if (cons->cur_x == CONSOLE_SIZE_X) {
                cons_newline(cons);
            }
        }
    }
    return;
}


void cons_newline(struct CONSOLE *cons)
{
    int x, y;
    struct SHEET *sheet = cons->sht;
    if (cons->cur_y < CONSOLE_SIZE_Y - CURSOR_HEIGHT) {
        cons->cur_y += CURSOR_HEIGHT;       /*  次の行へ。  */
    } else {
        /*  スクロール  */
        for (y = CURSOR_TOP; y < CONSOLE_SIZE_Y - CURSOR_HEIGHT; ++ y) {
            for (x = CURSOR_LEFT; x < CONSOLE_SIZE_X; ++ x){
                sheet->buf[x + y * sheet->bxsize] =
                        sheet->buf[x + (y + CURSOR_HEIGHT) * sheet->bxsize];
            }
        }
        boxfill8(sheet->buf, sheet->bxsize, COL8_000000,
                 CURSOR_LEFT, CONSOLE_SIZE_Y - CURSOR_HEIGHT,
                 CONSOLE_SIZE_X - 1, CONSOLE_SIZE_Y - 1);
        sheet_refresh(sheet, CURSOR_LEFT, CURSOR_TOP,
                      CONSOLE_SIZE_X, CONSOLE_SIZE_Y);
    }
    cons->cur_x = CURSOR_LEFT;
    return;
}

void cons_putstr0(struct CONSOLE *cons, char *s)
{
    for (; *s != 0; ++ s) {
        cons_putchar(cons, *s, 1);
    }
    return;
}

void cons_putstr1(struct CONSOLE *cons, char *s, int l)
{
    int i;
    for (i = 0; i < l; ++ i) {
        cons_putchar(cons, s[i], 1);
    }
    return;
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat,
                  unsigned int memtotal)
{
    if (strcmp(cmdline, "mem") == 0) {
        cmd_mem(cons, memtotal);
    } else if (strcmp(cmdline, "cls") == 0) {
        cmd_cls(cons);
    } else if (strcmp(cmdline, "dir") == 0) {
        cmd_dir(cons);
    } else if (strncmp(cmdline, "type ", 5) == 0) {
        cmd_type(cons, fat, cmdline);
    } else if (cmdline[0] != 0) {
        if (cmd_app(cons, fat, cmdline) == 0) {
            /*  コマンドではなく、アプリでもなく、さらに空行でもない。  */
            cons_putstr0(cons, "Bad command.\n\n");
        }
    }
    return;
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    char s[60];

    snprintf(s, sizeof(s), "total   %dMB\nfree %dKB\n\n",
             memtotal / (1024 * 1024), memman_total(memman) / 1024);
    cons_putstr0(cons, s);
    return;
}

void cmd_cls(struct CONSOLE *cons)
{
    struct SHEET *sheet = cons->sht;

    boxfill8(sheet->buf, sheet->bxsize, COL8_000000,
             CURSOR_LEFT, CURSOR_TOP,
             CONSOLE_SIZE_X - 1, CONSOLE_SIZE_Y - 1);
    sheet_refresh(sheet, CURSOR_LEFT, CURSOR_TOP,
                  CONSOLE_SIZE_X, CONSOLE_SIZE_Y);
    cons->cur_y = CURSOR_TOP;
    return;
}

void cmd_dir(struct CONSOLE *cons)
{
    struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
    int i, j;
    char s[30];

    for (i = 0; i < 224; ++ i) {
        if (finfo[i].name[0] == 0x00) {
            break;
        }
        if (finfo[i].name[0] == 0xe5) {
            continue;
        }
        if((finfo[i].type & 0x18) == 0) {
            snprintf(s, sizeof(s), "filename.ext   %7d\n", finfo[i].size);
            for (j = 0; j < 8; ++ j) {
                s[j] = finfo[i].name[j];
            }
            s[ 9] = finfo[i].ext[0];
            s[10] = finfo[i].ext[1];
            s[11] = finfo[i].ext[2];
            cons_putstr0(cons, s);
        }
    }
    cons_newline(cons);
    return;
}

void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct FILEINFO *finfo;
    char *p;
    int i;

    finfo = file_search(cmdline + 5,
                        (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);

    if (finfo != 0) {
        /*  ファイルが見つかった場合。  */
        p = (char *) memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat,
                      (char *) (ADR_DISKIMG + 0x003e00));
        cons_putstr1(cons, p, finfo->size);
        memman_free_4k(memman, (int)p, finfo->size);
    } else {
        /*  ファイルが見つからなかった場合  */
        cons_putstr0(cons, "File not found.\n");
    }
    cons_newline(cons);
    return;
}

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct FILEINFO *finfo;
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    char name[18], *p, *q;
    struct TASK *task = task_now();
    int i, segsiz, datsiz, esp, dathrb;

    /*  コマンドラインからファイル名を生成  */
    for (i = 0; i < 13; ++ i) {
        if (cmdline[i] <= ' ') {
            break;
        }
        name[i] = cmdline[i];
    }
    name[i] = 0;

    /*  ファイルを探す  */
    finfo = file_search(name,
                        (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    if (finfo == 0 && name[i - 1] != '.') {
        /*  見つからなかったので後ろに .HRB をつけてもう一度探してみる  */
        name[i   ]  = '.';
        name[i + 1] = 'H';
        name[i + 2] = 'R';
        name[i + 3] = 'B';
        name[i + 4] = 0;
        finfo = file_search(name,
                        (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    }

    if (finfo != 0) {
        /*  ファイルが見つかった場合。  */
        p = (char *) memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat,
                      (char *)(ADR_DISKIMG + 0x003e00));
        if (finfo->size >= 36 && strncmp(p + 4, "Hari", 4) == 0
                && *p == 0x00)
        {
            segsiz = *((int *) (p + 0x0000));
            esp    = *((int *) (p + 0x000c));
            datsiz = *((int *) (p + 0x0010));
            dathrb = *((int *) (p + 0x0014));
            q = (char *) memman_alloc_4k(memman, segsiz);
            *((int *) 0xfe8) = (int) q;
            set_segmdesc(gdt + 1003, finfo->size - 1,
                         (int) p, AR_CODE32_ER + 0x60);
            set_segmdesc(gdt + 1004, segsiz - 1,
                         (int) q, AR_DATA32_RW + 0x60);
            for (i = 0; i < datsiz; ++ i) {
                q[esp + i] = p[dathrb + i];
            }
            start_app(0x1b, 1003 * 8, esp, 1004 * 8, &(task->tss.esp0));
            memman_free_4k(memman, (int) q, segsiz);
        } else {
            cons_putstr0(cons, ".hrb file format error.\n");
        }
        memman_free_4k(memman, (int) p, finfo->size);
        cons_newline(cons);
        return 1;
    }

    /*  ファイルが見つからなかった場合  */
    return 0;
}

int *hrb_api(int edi, int esi, int ebp, int esp,
             int ebx, int edx, int ecx, int eax)
{
    int ds_base = *((int *) 0xfe8);
    struct TASK *task = task_now();
    struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
    struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
    struct SHEET *sht;
    volatile int *reg = &eax + 1;

    if (edx == 1) {
        cons_putchar(cons, eax & 0xff, 1);
    } else if (edx == 2) {
        cons_putstr0(cons, (char *) ebx + ds_base);
    } else if (edx == 3) {
        cons_putstr1(cons, (char *) ebx + ds_base, ecx);
    } else if (edx == 4) {
        return &(task->tss.esp0);
    } else if (edx == 5) {
        sht = sheet_alloc(shtctl);
        sheet_setbuf(sht, (char *) ebx + ds_base, esi, edi, eax);
        make_window8((char *) ebx + ds_base, esi, edi,
                     (char *) ecx + ds_base, 0);
        sheet_slide(sht, 100, 50);
        sheet_updown(sht, 3);
        reg[7] = (int )sht;
    } else if (edx == 6) {
        sht = (struct SHEET *) ebx;
        putfonts8_asc(sht->buf, sht->bxsize, esi, edi, eax,
                      (char *) ebp + ds_base);
        sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
    } else if (edx == 7) {
        sht = (struct SHEET *) ebx;
        boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
        sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
    } else if (edx == 8) {
        memman_init((struct MEMMAN *) (ebx + ds_base));
        ecx &= 0xfffffff0;
        memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
    }
    return 0;
}

int *inthandler0c(int *esp)
{
    struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
    struct TASK *task = task_now();
    char s[30];

    cons_putstr0(cons, "\nINT 0C :\n Stack Exception.\n");
    snprintf(s, sizeof(s), "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);

    return &(task->tss.esp0);   /*  異常終了させる  */
}

int *inthandler0d(int *esp)
{
    struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
    struct TASK *task = task_now();
    char s[30];

    cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
    snprintf(s, sizeof(s), "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);

    return &(task->tss.esp0);   /*  異常終了させる  */
}
