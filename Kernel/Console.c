/*  コンソール関係  */

#include "BootPack.h"
#include "../Common/stdio.h"
#include "../Common/string.h"


void console_task(struct SHEET *sheet, unsigned int memtotal)
{
    struct TASK *task = task_now();
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    int i;
    int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
    struct CONSOLE cons;
    struct FILEHANDLE fhandle[8];
    char cmdline[30];
    unsigned char *nihongo = (char *) *((int *) ADR_NIHONGO_FONT);
    struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);

    cons.sht = sheet;
    cons.cur_x = CURSOR_LEFT;
    cons.cur_y = CURSOR_TOP;
    cons.cur_c = -1;
    task->cons = &cons;
    task->cmdline = cmdline;

    if (cons.sht != 0) {
        cons.timer = timer_alloc();
        timer_init(cons.timer, &task->fifo, 1);
        timer_settime(cons.timer, 50);
    }
    file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));
    for (i = 0; i < 8; ++ i) {
        fhandle[i].buf = 0;     /*  未使用マーク。  */
    }
    task->fhandle = fhandle;
    task->fat = fat;
    if (nihongo[4096] != 0xff) {
        task->langmode = 1;
    } else {
        task->langmode = 0;
    }
    task->langbyte1 = 0;

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
            if (i <= 1 && cons.sht != 0) {      /*  カーソル用タイマ。  */
                if (i != 0) {
                    timer_init(cons.timer, &task->fifo, 0);
                    if (cons.cur_c >= 0) {
                        cons.cur_c = COL8_FFFFFF;
                    }
                } else {
                    timer_init(cons.timer, &task->fifo, 1);
                    if (cons.cur_c >= 0) {
                        cons.cur_c = COL8_000000;
                    }
                }
                timer_settime(cons.timer, 50);
            }
            if (i == 2) {       /*  カーソル ON */
                cons.cur_c = COL8_FFFFFF;
            }
            if (i == 3) {       /*  カーソル OFF    */
                if (cons.sht != 0) {
                    boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_000000,
                             cons.cur_x, cons.cur_y,
                             cons.cur_x + CURSOR_WIDTH  - 1,
                             cons.cur_y + CURSOR_HEIGHT - 1);
                }
                cons.cur_c = -1;
            }
            if (i == 4) {
                cmd_exit(&cons, fat);
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
                    if (cons.sht == 0) {
                        cmd_exit(&cons, fat);
                    }
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
            if (cons.sht != 0) {
                if (cons.cur_c >= 0) {
                    boxfill8(cons.sht->buf, cons.sht->bxsize, cons.cur_c,
                             cons.cur_x, cons.cur_y,
                             cons.cur_x + CURSOR_WIDTH  - 1,
                             cons.cur_y + CURSOR_HEIGHT - 1);
                }
                sheet_refresh(cons.sht, cons.cur_x, cons.cur_y,
                              cons.cur_x + CURSOR_WIDTH,
                              cons.cur_y + CURSOR_HEIGHT);
            }
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
            if (cons->sht != 0) {
                putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y,
                                  COL8_FFFFFF, COL8_000000, " ", 1);
            }
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
        if (cons->sht != 0) {
            putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y,
                              COL8_FFFFFF, COL8_000000, s, 1);
        }
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
    struct TASK *task = task_now();

    if (cons->cur_y < CONSOLE_SIZE_Y - CURSOR_HEIGHT) {
        cons->cur_y += CURSOR_HEIGHT;       /*  次の行へ。  */
    } else if (sheet != 0) {
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
    if (task->langmode == 1 && task->langbyte1 != 0) {
        cons->cur_x += CURSOR_WIDTH;
    }

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
    if (strcmp(cmdline, "mem") == 0 && cons->sht != 0) {
        cmd_mem(cons, memtotal);
    } else if (strcmp(cmdline, "cls") == 0 && cons->sht != 0) {
        cmd_cls(cons);
    } else if (strcmp(cmdline, "dir") == 0 && cons->sht != 0) {
        cmd_dir(cons);
    } else if (strcmp(cmdline, "exit") == 0) {
        cmd_exit(cons, fat);
    } else if (strncmp(cmdline, "start ", 6) == 0) {
        cmd_start(cons, cmdline, memtotal);
    } else if (strncmp(cmdline, "ncst ", 5) == 0) {
        cmd_ncst(cons, cmdline, memtotal);
    } else if (strncmp(cmdline, "langmode ", 9) == 0) {
        cmd_langmode(cons, cmdline);
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

void cmd_exit(struct CONSOLE *cons, int *fat)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct TASK *task = task_now();
    struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) ADR_SHT_CTL);
    struct FIFO32 *fifo = (struct FIFO32 *) *((int *) ADR_SYS_FIFO);

    if (cons->sht != 0) {
        timer_cancel(cons->timer);
    }
    memman_free_4k(memman, (int) fat, 4 * 2880);

    io_cli();
    if (cons->sht != 0) {
        fifo32_put(fifo, cons->sht - shtctl->sheets0 + 768);
    } else {
        fifo32_put(fifo, task - taskctl->tasks0 + 1024);
    }
    io_sti();

    for(;;) {
        task_sleep(task);
    }
}

void cmd_start(struct CONSOLE *cons, char *cmdline, int memtotal)
{
    struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) ADR_SHT_CTL);
    struct SHEET *sht = open_console(shtctl, memtotal);
    struct FIFO32 *fifo = &sht->task->fifo;
    int i;
    sheet_slide(sht, 32, 4);
    sheet_updown(sht, shtctl->top);

    /*  コマンドラインに入力された文字列を、新しいコンソールに入力  */
    for (i = 6; cmdline[i] != 0; ++ i) {
        fifo32_put(fifo, cmdline[i] + 256);
    }
    fifo32_put(fifo, 10 + 256);     /*  Enter.  */
    cons_newline(cons);

    return;
}

void cmd_ncst(struct CONSOLE *cons, char *cmdline, int memtotal)
{
    struct TASK *task = open_constask(0, memtotal);
    struct FIFO32 *fifo = &task->fifo;
    int i;

    /*  コマンドラインに入力された文字列を、新しいコンソールに入力  */
    for (i = 5; cmdline[i] != 0; ++ i) {
        fifo32_put(fifo, cmdline[i] + 256);
    }
    fifo32_put(fifo, 10 + 256);     /*  Enter.  */
    cons_newline(cons);

    return;
}

void cmd_langmode(struct CONSOLE *cons, char *cmdline)
{
    struct TASK *task = task_now();
    unsigned char mode = cmdline[9] - '0';
    if (mode <= 2) {
        task->langmode = mode;
    } else {
        cons_putstr0(cons, "mode number error.\n");
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
    struct SHTCTL *shtctl;
    struct SHEET *sht;

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
            task->ds_base = (int) q;
            set_segmdesc(task->ldt + 0, finfo->size - 1,
                         (int) p, AR_CODE32_ER + 0x60);
            set_segmdesc(task->ldt + 1, segsiz - 1,
                         (int) q, AR_DATA32_RW + 0x60);
            for (i = 0; i < datsiz; ++ i) {
                q[esp + i] = p[dathrb + i];
            }
            start_app(0x1b, 0 * 8 + 4, esp, 1 * 8 + 4, &(task->tss.esp0));
            shtctl = (struct SHTCTL *) *((int *) ADR_SHT_CTL);
            for (i = 0; i < MAX_SHEETS; ++ i) {
                sht = &(shtctl->sheets0[i]);
                if ((sht->flags & 0x11) == 0x11 && sht->task == task) {
                    /*  アプリが開きっぱなしにした下じきを発見  */
                    sheet_free(sht);
                }
            }
            for (i = 0; i < 8; ++ i) {
                /*  クローズしてないファイルをクローズ  */
                if (task->fhandle[i].buf != 0) {
                    memman_free_4k(memman, (int) task->fhandle[i].buf,
                                   task->fhandle[i].size);
                    task->fhandle[i].buf = 0;
                }
            }
            timer_cancelall(&task->fifo);
            memman_free_4k(memman, (int) q, segsiz);
            task->langbyte1 = 0;
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


void hrb_api_013_linewin(
        int edi, int esi, int ebp, int ebx, int ecx, int eax)
{
    int tmp;
    struct SHEET *sht = (struct SHEET *) (ebx & 0xfffffffe);

    hrb_api_linewin(sht, eax, ecx, esi, edi, ebp);
    if ((ebx & 1) == 0) {
        if (eax > esi) {
            tmp = eax;
            eax = esi;
            esi = tmp;
        }
        if (ecx > edi) {
            tmp = ecx;
            ecx = edi;
            edi = tmp;
        }
        sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
    }
}

int hrb_api_015_getkey(int eax, struct TASK *task)
{
    struct CONSOLE *cons = task->cons;
    struct FIFO32 *sys_fifo = (struct FIFO32 *) *((int *) ADR_SYS_FIFO);
    struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) ADR_SHT_CTL);
    int i;

    for (;;) {
        io_cli();
        if (fifo32_status(&task->fifo) == 0) {
            if (eax != 0) {
                task_sleep(task);   /*  FIFOが空なので寝て待つ  */
            } else {
                io_sti();
                return  -1;
            }
        }

        i = fifo32_get(&task->fifo);
        io_sti();
        if (i <= 1) {   /*  カーソル用タイマ。  */
            /*  アプリ実行中はカーソルが出ないので、
            いつも次は表示用の 1を注文しておく  */
            timer_init(cons->timer, &task->fifo, 1);
            timer_settime(cons->timer, 50);
        }
        if (i == 2) {   /*  カーソル ON */
            cons->cur_c = COL8_FFFFFF;
        }
        if (i == 3) {   /*  カーソル OFF    */
            cons->cur_c = -1;
        }
        if (i == 4) {   /*  コンソールだけを閉じる  */
            timer_cancel(cons->timer);
            io_cli();
            fifo32_put(sys_fifo, cons->sht - shtctl->sheets0 + 2024);
            cons->sht = 0;
            io_sti();
        }
        if (i >= 256) {     /*  キーボードデータなど。  */
            return  i - 256;
        }
    }

    return  0;
}

void  hrb_api_020_beep(int eax)
{
    int i;

    if (eax == 0) {
        i = io_in8(0x61);
        io_out8(0x61, i & 0x0d);
    } else {
        i = 1193180000 / eax;
        io_out8(0x43, 0xb6);
        io_out8(0x42, i & 0xff);
        io_out8(0x42, i >> 8);
        i = io_in8(0x61);
        io_out8(0x61, (i | 0x03) & 0x0f);
    }
}

int hrb_api_021_fopen(int ebx, struct TASK *task)
{
    const int ds_base = task->ds_base;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    int i, reg7;
    struct FILEINFO *finfo;
    struct FILEHANDLE *fh;

    for (i = 0; i < 8; ++ i) {
        if (task->fhandle[i].buf == 0) {
            break;
        }
    }
    fh = &task->fhandle[i];
    reg7 = 0;
    if (i < 8) {
        finfo = file_search(
                    (char *) ebx + ds_base,
                    (struct FILEINFO *) (ADR_DISKIMG + 0x002600),
                    224);
        if (finfo != 0) {
            reg7 = (int) fh;
            fh->buf = (char *) memman_alloc_4k(memman, finfo->size);
            fh->size = finfo->size;
            fh->pos = 0;
            file_loadfile(finfo->clustno, finfo->size, fh->buf,
                          task->fat, (char *) (ADR_DISKIMG + 0x003e00));
        }
    }
    return  reg7;
}

int hrb_api_023_fseek(int ebx, int ecx, int eax)
{
    struct FILEHANDLE *fh = (struct FILEHANDLE *) eax;
    if (ecx == 0) {
        fh->pos = ebx;
    } else if (ecx == 1) {
        fh->pos += ebx;
    } else if (ecx == 2) {
        fh->pos = fh->size + ebx;
    }
    if (fh->pos < 0) {
        fh->pos = 0;
    }
    if (fh->pos > fh->size) {
        fh->pos = fh->size;
    }

    return  0;
}

int hrb_api_024_fsize(int ecx, int eax)
{
    struct FILEHANDLE *fh = (struct FILEHANDLE *) eax;
    int reg7;
    if (ecx == 0) {
        reg7 = fh->size;
    } else if (ecx == 1) {
        reg7 = fh->pos;
    } else if (ecx == 2) {
        reg7 = fh->pos - fh->size;
    }
    return  reg7;
}

int hrb_api_025_fread(int ebx, int ecx, int eax, int ds_base)
{
    struct FILEHANDLE *fh = (struct FILEHANDLE *) eax;
    int i;

    for (i = 0; i < ecx; ++ i) {
        if (fh->pos == fh->size) {
            break;
        }
        *((char *) ebx + ds_base + i) = fh->buf[fh->pos];
        ++ fh->pos;
    }
    return  i;
}

int hrb_api_026_cmdline(int ebx, int ecx, struct TASK *task)
{
    const int ds_base = task->ds_base;
    int i = 0;
    for (;;) {
        *((char *) ebx + ds_base + i) = task->cmdline[i];
        if (task->cmdline[i] == 0) {
            break;
        }
        if (i >= ecx) {
            break;
        }
        ++ i;
    }
    return  i;
}

int *hrb_api(int edi, int esi, int ebp, int esp,
             int ebx, int edx, int ecx, int eax)
{
    struct TASK *task = task_now();
    const int ds_base = task->ds_base;
    struct CONSOLE *cons = task->cons;
    struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) ADR_SHT_CTL);
    struct SHEET *sht;
    struct TIMER *timer;
    volatile int *reg = &eax + 1;
    struct FILEHANDLE *fh;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

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
        sht->task = task;
        sht->flags |= 0x10;
        sheet_setbuf(sht, (char *) ebx + ds_base, esi, edi, eax);
        make_window8((char *) ebx + ds_base, esi, edi,
                     (char *) ecx + ds_base, 0);
        sheet_slide(sht, ((shtctl->xsize - esi) / 2) & ~3,
                    (shtctl->ysize - edi) / 2);
        sheet_updown(sht, shtctl->top);
        reg[7] = (int) sht;
    } else if (edx == 6) {
        sht = (struct SHEET *) (ebx & 0xfffffffe);
        putfonts8_asc(sht->buf, sht->bxsize, esi, edi, eax,
                      (char *) ebp + ds_base);
        if ((ebx & 1) == 0) {
            sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
        }
    } else if (edx == 7) {
        sht = (struct SHEET *) (ebx & 0xfffffffe);
        boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
        if ((ebx & 1) == 0) {
            sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
        }
    } else if (edx == 8) {
        memman_init((struct MEMMAN *) (ebx + ds_base));
        ecx &= 0xfffffff0;
        memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
    } else if (edx == 9) {
        ecx = (ecx + 0x0f) & 0xfffffff0;
        reg[7] = memman_alloc((struct MEMMAN *) (ebx + ds_base), ecx);
    } else if (edx == 10) {
        ecx = (ecx + 0x0f) & 0xfffffff0;
        memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
    } else if (edx == 11) {
        sht = (struct SHEET *) (ebx & 0xfffffffe);
        sht->buf[sht->bxsize * edi + esi] = eax;
        if ((ebx & 1) == 0) {
            sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
        }
    } else if (edx == 12) {
        sht = (struct SHEET *) ebx;
        sheet_refresh(sht, eax, ecx, esi, edi);
    } else if (edx == 13) {
        hrb_api_013_linewin(edi, esi, ebp, ebx, ecx, eax);
    } else if (edx == 14) {
        sheet_free((struct SHEET *) ebx);
    } else if (edx == 15) {
        reg[7] = hrb_api_015_getkey(eax, task);
    } else if (edx == 16) {
        timer = timer_alloc();
        timer->flags2 = 1;      /*  自動キャンセル有効  */
        reg[7] = (int) timer;
    } else if (edx == 17) {
        timer_init((struct TIMER *) ebx, &task->fifo, eax + 256);
    } else if (edx == 18) {
        timer_settime((struct TIMER *) ebx, eax);
    } else if (edx == 19) {
        timer_free((struct TIMER *) ebx);
    } else if (edx == 20) {
        hrb_api_020_beep(eax);
    } else if (edx == 21) {
        reg[7] = hrb_api_021_fopen(ebx, task);
    } else if (edx == 22) {
        fh = (struct FILEHANDLE *) eax;
        memman_free_4k(memman, (int) fh->buf, fh->size);
        fh->buf = 0;
    } else if (edx == 23) {
        hrb_api_023_fseek(ebx, ecx, eax);
    } else if (edx == 24) {
        reg[7] = hrb_api_024_fsize(ecx, eax);
    } else if (edx == 25) {
        reg[7] = hrb_api_025_fread(ebx, ecx, eax, ds_base);
    } else if (edx == 26) {
        reg[7] = hrb_api_026_cmdline(ebx, ecx, task);
    } else if (edx == 27) {
        reg[7] = task->langmode;
    }

    return 0;
}

int *inthandler0c(int *esp)
{
    struct TASK *task = task_now();
    struct CONSOLE *cons = task->cons;
    char s[30];

    cons_putstr0(cons, "\nINT 0C :\n Stack Exception.\n");
    snprintf(s, sizeof(s), "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);

    return &(task->tss.esp0);   /*  異常終了させる  */
}

int *inthandler0d(int *esp)
{
    struct TASK *task = task_now();
    struct CONSOLE *cons = task->cons;
    char s[30];

    cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
    snprintf(s, sizeof(s), "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);

    return &(task->tss.esp0);   /*  異常終了させる  */
}

void hrb_api_linewin(
        struct SHEET *sht, int x0, int y0, int x1, int y1, int col)
{
    int i, x, y, len, dx, dy;

    dx = x1 - x0;
    dy = y1 - y0;
    x = x0 << 10;
    y = y0 << 10;
    if (dx < 0) {
        dx = - dx;
    }
    if (dy < 0) {
        dy = - dy;
    }
    if (dx > dy) {
        len = dx + 1;
        if (x0 > x1) {
            dx = -1024;
        } else {
            dx =  1024;
        }
        if (y0 <= y1) {
            dy = ((y1 - y0 + 1) << 10) / len;
        } else {
            dy = ((y1 - y0 - 1) << 10) / len;
        }
    } else {
        len = dy + 1;
        if (y0 > y1) {
            dy = -1024;
        } else {
            dy =  1024;
        }
        if (x0 <= x1) {
            dx = ((x1 - x0 + 1) << 10) / len;
        } else {
            dx = ((x1 - x0 - 1) << 10) / len;
        }
    }

    for (i = 0; i < len; ++ i) {
        sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
        x += dx;
        y += dy;
    }

    return;
}
