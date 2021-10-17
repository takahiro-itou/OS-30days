/*  bootpack  のメイン  */

#include "BootPack.h"
#include "../Common/stdio.h"


void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *)(0xff0);
    char s[40], mcursor[256], keybuf[32], mousebuf[128];
    int mx, my, i;
    unsigned int memtotal;
    struct MOUSE_DEC mdec;
    struct MEMMAN *memman = (struct MEMMAN *)(MEMMAN_ADDR);

    init_gdtidt();
    init_pic();
    io_sti();

    fifo8_init(&keyfifo, sizeof(keybuf), keybuf);
    fifo8_init(&mousefifo, sizeof(mousebuf), mousebuf);
    io_out8(PIC0_IMR, 0xf9);    /*  PIC1とキーボードを許可  */
    io_out8(PIC1_IMR, 0xef);    /*  マウスを許可            */

    init_keyboard();
    enable_mouse(&mdec);

    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    snprintf(s, sizeof(s) - 1, "(%3d, %3d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

    snprintf(s, sizeof(s) - 1, "memory %dMB   free : %dKB",
             memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

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

#define EFLAGS_AC_BIT       0x00040000
#define CR0_CACHE_DISABLE   0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
    char flg486 = 0;
    unsigned int eflg, cr0, i;

    /*  386か 486 以降なのかの確認  */
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;
    io_store_eflags(eflg);

    eflg = io_load_eflags();
    if ((eflg & EFLAGS_AC_BIT) != 0) {
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflg);

    if (flg486 != 0) {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;   /*  キャッシュ禁止  */
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if (flg486 != 0) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;  /*  キャッシュ許可  */
        store_cr0(cr0);
    }

    return i;
}

#if 0
unsigned int memtest_sub(unsigned int start, unsigned int end)
{
    unsigned int i, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
    volatile unsigned int *p;

    for (i = start; i <= end; i += 0x1000) {
        p = (unsigned int *)(i + 0x0ffc);
        old = *p;           /*  いじる前の値を覚えておく。  */
        *p = pat0;          /*  ためしに書いてみる。        */
        *p ^= 0xffffffff;   /*  そしてそれを反転してみる。  */
        if (*p != pat1) {
not_memory:
            *p = old;
            break;
        }
        *p ^= 0xffffffff;   /*  もう一度反転してみる。      */
        if (*p != pat0) {
            goto not_memory;
        }
        *p = old;           /*  いじった値を元に戻す。      */
    }
    return i;
}
#endif

void memman_init(struct MEMMAN *man)
{
    man->frees      = 0;    /*  あき情報の個数。    */
    man->maxfrees   = 0;    /*  状況観察用。frees の最大値。    */
    man->lostsize   = 0;    /*  解放に失敗した合計サイズ。      */
    man->losts      = 0;    /*  解放に失敗した回数。            */
    return;
}

unsigned int memman_total(struct MEMMAN *man)
{
    unsigned int i, t = 0;
    for (i = 0; i < man->frees; ++ i) {
        t += man->free[i].size;
    }
    return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
    unsigned int i, a;
    for (i = 0; i < man->frees; ++ i) {
        if (man->free[i].size >= size) {
            /*  十分な広さのあきを発見  */
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if (man->free[i].size == 0) {
                /*  free[i] がなくなったので前へつめる  */
                -- man->frees;
                for (; i < man->frees; ++ i) {
                    man->free[i] = man->free[i + 1];
                }
            }
            return a;
        }
    }
    return 0;   /*  あきがない  */
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
    int i, j;

    /*  まとめやすさを考えると free[] が addr 順に並んでいるほうがいい
    *   だからまず、どこに入れるべきかを決める  */
    for (i = 0; i < man->frees; ++ i) {
        if (man->free[i].addr > addr) {
            break;
        }
    }
    /*  free[i - 1].addr < addr < free[i].addr  */
    if (i > 0) {
        /*  前がある。  */
        if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
            /*  前のあき領域にまとめられる  */
            man->free[i - 1].size += size;
            if (i < man->frees) {
                /*  後ろもある  */
                if (addr + size == man->free[i].addr) {
                    /*  なんと後ろともまとめられる  */
                    man->free[i - 1].size += man->free[i].size;
                    /*  man->free[i]  の削除    */
                    /*  free[i] がなくなったので前へつめる  */
                    -- man->frees;
                    for ( ; i < man->frees; ++ i) {
                        man->free[i] = man->free[i + 1];
                    }
                }
            }
            return 0;
        }
    }
    /*  前とはまとめられなかった。  */
    if (i < man->frees) {
        /*  後ろがある  */
        if (addr + size == man->free[i].addr) {
            /*  後ろとはまとめられる。  */
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0;
        }
    }
    /*  前にも後ろにもまとめられない。  */
    if (man->frees < MEMMAN_FREES) {
        /*  free[i] より後ろを、後ろへずらして、すきまと作る。  */
        for (j = man->frees; j > i; -- j) {
            man->free[j] = man->free[j - 1];
        }
        ++ man->frees;
        if (man->maxfrees < man->frees) {
            man->maxfrees = man->frees;     /*  最大値を更新。  */
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }

    /*  後ろにずらせなかった。  */
    ++ man->losts;
    man->lostsize += size;
    return -1;
}
