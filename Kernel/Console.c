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
    //int cursor_x = 16, cursor_y = 28, cursor_c = -1;
    char s[30], cmdline[30], *p;
    int x, y;
    struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;

    cons.sht = sheet;
    cons.cur_x =  8;
    cons.cur_y = 28;
    cons.cur_c = -1;

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
                         cons.cur_x + 7, cons.cur_y + 15);
                cons.cur_c = -1;
            }
            if (256 <= i && i <= 511) {
                /*  キーボードデータ。  */
                if (i == 8 + 256) {
                    /*  バックスペース  */
                    if (cons.cur_x > 16) {
                        putfonts8_asc_sht(sheet, cons.cur_x, cons.cur_y,
                                          COL8_FFFFFF, COL8_000000, " ", 1);
                        cons.cur_x -= 8;
                    }
                } else if (i == 10 + 256) {
                    /*  Enter.  */
                    /*  カーソルをスペースで消す。  */
                    putfonts8_asc_sht(sheet, cons.cur_x, cons.cur_y,
                                      COL8_FFFFFF, COL8_000000, " ", 1);
                    cmdline[cons.cur_x / 8 - 2] = 0;
                    cons_newline(&cons);

                    /*  コマンド実行。  */
                    if (strcmp(cmdline, "mem") == 0) {
                        /*  mem コマンド。  */
                        snprintf(s, sizeof(s), "total   %dMB",
                                 memtotal / (1024 * 1024));
                        putfonts8_asc_sht(sheet, 8, cons.cur_y, COL8_FFFFFF,
                                          COL8_000000, s, 30);
                        cons_newline(&cons);
                        snprintf(s, sizeof(s), "free %dKB",
                                 memman_total(memman) / 1024);
                        putfonts8_asc_sht(sheet, 8, cons.cur_y, COL8_FFFFFF,
                                          COL8_000000, s, 30);
                        cons_newline(&cons);
                        cons_newline(&cons);
                    } else if (strcmp(cmdline, "cls") == 0) {
                        boxfill8(sheet->buf, sheet->bxsize, COL8_000000,
                                 8, 28, 8 + 240 - 1, 28 + 128 - 1);
                        sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
                        cons.cur_y = 28;
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
                                putfonts8_asc_sht(sheet, 8, cons.cur_y,
                                                  COL8_FFFFFF,
                                                  COL8_000000,
                                                  s, 30);
                                cons_newline(&cons);
                            }
                        }
                        cons_newline(&cons);
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
                            cons.cur_x = 8;
                            for (y = 0; y < finfo[x].size; ++ y) {
                                s[0] = p[y];
                                s[1] = 0;
                                if (s[0] == 0x09) {
                                    for(;;) {
                                        putfonts8_asc_sht(sheet, cons.cur_x,
                                                          cons.cur_y,
                                                          COL8_FFFFFF,
                                                          COL8_000000,
                                                          " ", 1);
                                        cons.cur_x += 8;
                                        if (cons.cur_x == 8 + 240) {
                                            cons.cur_x = 8;
                                            cons_newline(&cons);
                                        }
                                        if (((cons.cur_x - 8) & 0x1f) == 0) {
                                            break;
                                        }
                                    }
                                } else if (s[0]== 0x0a) {
                                    cons.cur_x = 8;
                                    cons_newline(&cons);
                                } else if (s[0] == 0x0d) {
                                } else {
                                    putfonts8_asc_sht(sheet,
                                                  cons.cur_x, cons.cur_y,
                                                  COL8_FFFFFF, COL8_000000,
                                                  s, 1);
                                    cons.cur_x += 8;
                                    if (cons.cur_x == 8 + 240) {
                                        cons.cur_x = 8;
                                        cons_newline(&cons);
                                    }
                                }
                            }
                            memman_free_4k(memman, (int)p, finfo[x].size);
                        } else {
                            /*  ファイルが見つからなかった場合  */
                            putfonts8_asc_sht(sheet, 8, cons.cur_y,
                                              COL8_FFFFFF, COL8_000000,
                                              "File not found.", 15);
                            cons_newline(&cons);
                        }
                        cons_newline(&cons);
                    } else if (strcmp(cmdline, "hlt") == 0) {
                        /*  hlt.hrb アプリケーションを起動  */
                        for (y = 0; y < 11; ++ y) {
                            s[y] = ' ';
                        }
                        s[0] = 'H';
                        s[1] = 'L';
                        s[2] = 'T';
                        s[8] = 'H';
                        s[9] = 'R';
                        s[10] = 'B';
                        for (x = 0; x < 224; ) {
                            if (finfo[x].name[0] == 0x00) {
                                break;
                            }
                            if ((finfo[x].type & 0x18) == 0) {
                                for (y = 0; y < 8; ++ y) {
                                    if (finfo[x].name[y] != s[y]) {
                                        goto hlt_next_file;
                                    }
                                }
                                for (y = 0; y < 3; ++ y) {
                                    if (finfo[x].ext[y] != s[y + 8]) {
                                        goto hlt_next_file;
                                    }
                                }
                                break;
                            }
hlt_next_file:
                            ++ x;
                        }
                        if (x < 224 && finfo[x].name[0] != 0x00) {
                            /*  ファイルが見つかった場合。  */
                            p = (char *) memman_alloc_4k(memman, finfo[x].size);
                            file_loadfile(finfo[x].clustno, finfo[x].size,
                                          p, fat,
                                          (char *)(ADR_DISKIMG + 0x003e00));
                            set_segmdesc(gdt + 1003, finfo[x].size - 1,
                                         (int) p, AR_CODE32_ER);
                            farjmp(0, 1003 * 8);
                            memman_free_4k(memman, (int) p, finfo[x].size);
                        } else {
                            /*  ファイルが見つからなかった場合  */
                            putfonts8_asc_sht(sheet, 8, cons.cur_y,
                                              COL8_FFFFFF, COL8_000000,
                                              "File not found.", 15);
                            cons_newline(&cons);
                        }
                        cons_newline(&cons);
                    } else if (cmdline[0] != 0) {
                        /*  コマンドではなく、さらに空行でもない。  */
                        putfonts8_asc_sht(sheet, 8, cons.cur_y, COL8_FFFFFF,
                                          COL8_000000, "Bad command.", 12);
                        cons_newline(&cons);
                        cons_newline(&cons);
                    }
                    /*  プロンプト表示  */
                    putfonts8_asc_sht(sheet, 8, cons.cur_y,
                                      COL8_FFFFFF, COL8_000000, ">", 1);
                    cons.cur_x = 16;
                } else {
                    if (cons.cur_x < 240) {
                        s[0] = i - 256;
                        s[1] = 0;
                        cmdline[cons.cur_x / 8 - 2] = i - 256;
                        putfonts8_asc_sht(sheet, cons.cur_x, cons.cur_y,
                                          COL8_FFFFFF, COL8_000000, s, 1);
                        cons.cur_x += 8;
                    }
                }
            }
            /*  カーソル再表示  */
            boxfill8(sheet->buf, sheet->bxsize, cons.cur_c,
                     cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
            sheet_refresh(sheet, cons.cur_x, cons.cur_y,
                          cons.cur_x + 8, cons.cur_y + 16);
        }
    }
}

void cons_newline(struct CONSOLE *cons)
{
    int x, y;
    struct SHEET *sheet = cons->sht;
    if (cons->cur_y < 28 + 112) {
        cons->cur_y += 16;      /*  次の行へ。  */
    } else {
        /*  スクロール  */
        for (y = 28; y < 28 + 112; ++ y) {
            for (x = 8; x < 8 + 240; ++ x){
                sheet->buf[x + y * sheet->bxsize] =
                        sheet->buf[x + (y + 16) * sheet->bxsize];
            }
        }
        boxfill8(sheet->buf, sheet->bxsize, COL8_000000,
                 8, 28 + 112, 8 + 240 - 1, 28 + 128 - 1);
        sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
    }
    cons->cur_x = 8;
    return;
}
