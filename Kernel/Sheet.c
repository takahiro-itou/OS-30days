/*  重ね合わせ処理  */

#include "BootPack.h"

#define SHEET_USE       1

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram,
                           int xsize, int ysize)
{
    struct SHTCTL *ctl;
    int i;
    ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if (ctl == 0) {
        goto err;
    }
    ctl->map    = (unsigned char *)memman_alloc_4k(memman, xsize * ysize);
    if (ctl->map == 0) {
        memman_free_4k(memman, (int)ctl, sizeof(struct SHTCTL));
        goto err;
    }

    ctl->vram   = vram;
    ctl->xsize  = xsize;
    ctl->ysize  = ysize;
    ctl->top    = -1;
    for (i = 0; i < MAX_SHEETS; ++ i) {
        ctl->sheets0[i].flags   = 0;
        ctl->sheets0[i].ctl     = ctl;
    }
err:
    return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
    struct SHEET *sht;
    int i;
    for (i = 0; i < MAX_SHEETS; ++ i) {
        if (ctl->sheets0[i].flags == 0) {
            sht = &(ctl->sheets0[i]);
            sht->flags  = SHEET_USE;    /*  使用中マーク。  */
            sht->height = -1;           /*  非表示中。      */
            sht->task = 0;          /*  自動で閉じる機能を使わない  */
            return sht;
        }
    }
    return 0;   /*  すべてのシートが使用中だった。  */
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf,
                  int xsize, int ysize, int col_inv)
{
    sht->buf     = buf;
    sht->bxsize  = xsize;
    sht->bysize  = ysize;
    sht->col_inv = col_inv;
    return;
}


void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0,
                      int vx1, int vy1, int h0)
{
    int h, bx, by, vx, vy, bx0, by0, bx1, by1, sid4, *p;
    unsigned char *buf, sid, *map = ctl->map;
    struct SHEET *sht;

    /*  refresh 範囲が画面外にはみ出していたら補正  */
    if (vx0 < 0) { vx0 = 0; }
    if (vy0 < 0) { vy0 = 0; }
    if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
    if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }

    for (h = h0; h <= ctl->top; ++ h) {
        sht = ctl->sheets[h];
        sid = sht - ctl->sheets0;
        buf = sht->buf;

        /*  vx0～vy1を使って bx0～by1 を逆算する。  */
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        if (bx0 < 0) { bx0 = 0; }
        if (by0 < 0) { by0 = 0; }
        if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
        if (by1 > sht->bysize) { by1 = sht->bysize; }

        if (sht->col_inv == -1) {
            if ((sht->vx0 & 3) == 0 && (bx0 & 3) == 0 && (bx1 & 3) == 0) {
                /*  透明色なし専用の高速版 (4 バイト型) 。  */
                bx1 = (bx1 - bx0) / 4;
                sid4 = sid | sid << 8 | sid << 16 | sid << 24;
                for (by = by0; by < by1; ++ by) {
                    vy = sht->vy0 + by;
                    vx = sht->vx0 + bx0;
                    p = (int *) &map[vy * ctl->xsize + vx];
                    for (bx = 0; bx < bx1; ++ bx) {
                        p[bx] = sid4;
                    }
                }
            } else {
                /*  透明色なし専用の高速版 (1 バイト型) 。  */
                for (by = by0; by < by1; ++ by) {
                    vy = sht->vy0 + by;
                    for (bx = bx0; bx < bx1; ++ bx) {
                        vx = sht->vx0 + bx;
                        map[vy * ctl->xsize + vx] = sid;
                    }
                }
            }
        } else {
            /*  透明色ありの一般版  */
            for (by = by0; by < by1; ++ by) {
                vy = sht->vy0 + by;
                for (bx = bx0; bx < bx1; ++ bx) {
                    vx = sht->vx0 + bx;
                    if (buf[by * sht->bxsize + bx] != sht->col_inv) {
                        map[vy * ctl->xsize + vx] = sid;
                    }
                }
            }
        }
    }
    return;
}

#define     WRITE_VRAM(dx, dy, sx, sy)                                  \
if (map[(dy) * ctl->xsize + (dx)] == sid) {                             \
    vram[(dy) * ctl->xsize + (dx)] = buf[(sy) * sht->bxsize + (sy)];    \
}                                                                       \

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0,
                      int vx1, int vy1, int h0, int h1)
{
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;
    int bx2, sid4, i, i1, *p, *q, *r;
    unsigned char *buf, c, *vram = ctl->vram, *map = ctl->map, sid;
    struct SHEET *sht;

    /*  refresh 範囲が画面外にはみ出していたら補正  */
    if (vx0 < 0) { vx0 = 0; }
    if (vy0 < 0) { vy0 = 0; }
    if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
    if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }

    for (h = h0; h <= h1; ++ h) {
        sht = ctl->sheets[h];
        buf = sht->buf;
        sid = sht - ctl->sheets[0];

        /*  vx0～vy1を使って bx0～by1 を逆算する。  */
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        if (bx0 < 0) { bx0 = 0; }
        if (by0 < 0) { by0 = 0; }
        if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
        if (by1 > sht->bysize) { by1 = sht->bysize; }

        if ((sht->vx0 & 3) == 0) {
            /*  4 バイト型  */
            i  = (bx0 + 3) / 4;
            i1 =  bx1      / 4;
            i1 = i1 - i;
            sid4 = sid | sid << 8 | sid << 16 | sid << 24;
            for (by = by0; by < by1; ++ by) {
                vy = sht->vy0 + by;
                for (bx = bx0; bx < bx1 && (bx & 3) != 0; ++ bx) {
                    /*  前の端数を 1バイトずつ  */
                    vx = sht->vx0 + bx;
                    WRITE_VRAM(vx, vy, bx, by);
                }
                vx = sht->vx0 + bx;
                p = (int *) &map[vy * ctl->xsize + vx];
                q = (int *) &vram[vy * ctl->xsize + vx];
                r = (int *) &buf[by * sht->bxsize + bx];
                for (i = 0; i < i1; ++ i) {
                    if (p[i] == sid4) {
                        q[i] = r[i];
                    } else {
                        bx2 = bx + i * 4;
                        vx = sht->vx0 + bx2;
                        WRITE_VRAM(vx + 0, vy, bx2 + 0, by);
                        WRITE_VRAM(vx + 1, vy, bx2 + 1, by);
                        WRITE_VRAM(vx + 2, vy, bx2 + 2, by);
                        WRITE_VRAM(vx + 3, vy, bx2 + 3, by);
                    }
                }
                for (bx += i1 * 4; bx < bx1; ++ bx) {
                    /*  後ろの端数を 1バイトずつ。  */
                    vx = sht->vx0 + bx;
                    WRITE_VRAM(vx, vy, bx, by);
                }
            }
        } else {
            /*  1 バイト型  */
            for (by = by0; by < by1; ++ by) {
                vy = sht->vy0 + by;
                for (bx = bx0; bx < bx1; ++ bx) {
                    vx = sht->vx0 + bx;
                    WRITE_VRAM(vx, vy, bx, by);
                }
            }
        }
    }
    return;
}

#undef  WRITE_VRAM

void sheet_updown(struct SHEET *sht, int height)
{
    struct SHTCTL *ctl = sht->ctl;
    int h, old = sht->height;   /*  設定前の高さを記憶する  */

    /*  指定が低すぎや高すぎだったら、修正する  */
    if (height > ctl->top + 1) {
        height = ctl->top + 1;
    }
    if (height < -1) {
        height = -1;
    }
    sht->height = height;   /*  高さを設定  */

    /*  以下は主に sheets[] の並べ替え  */
    if (old > height) {     /*  以前よりも低くなる  */
        if (height >= 0) {
            /*  間のものを引き上げる。  */
            for (h = old; h > height; -- h) {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
            sheet_refreshmap(ctl, sht->vx0, sht->vy0,
                             sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize,
                             height + 1);
            sheet_refreshsub(ctl, sht->vx0, sht->vy0,
                             sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize,
                             height + 1, old);
        } else {
            /*  非表示  */
            if (ctl->top > old) {
                /*  上になっているものをおろす  */
                for (h = old; h < ctl->top; ++ h) {
                    ctl->sheets[h] = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            /*  表示中の下じきが１つ減るので、一番上の高さが減る。  */
            -- ctl->top;
            sheet_refreshmap(ctl, sht->vx0, sht->vy0,
                             sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize,
                             0);
            sheet_refreshsub(ctl, sht->vx0, sht->vy0,
                             sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize,
                             0, old - 1);
        }
    } else if (old < height) {  /*  以前よりも高くなる  */
        if (old >= 0) {
            /*  間のものを押し下げる。  */
            for (h = old; h < height; ++ h) {
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        } else {
            /*  非表示状態から表示状態へ。  */
            for (h = ctl->top; h >= height; -- h) {
                ctl->sheets[h + 1] = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            /*  表示中の下じきが１つ増えるので、一番上の高さが増える。  */
            ++ ctl->top;
        }
        sheet_refreshmap(ctl, sht->vx0, sht->vy0,
                         sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize,
                         height);
        sheet_refreshsub(ctl, sht->vx0, sht->vy0,
                         sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize,
                         height, height);
    }
    return;
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
    if (sht->height >= 0) {
        sheet_refreshsub(sht->ctl,
                         sht->vx0 + bx0, sht->vy0 + by0,
                         sht->vx0 + bx1, sht->vy0 + by1,
                         sht->height, sht->height);
    }
    return;
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if (sht->height >= 0) {
        sheet_refreshmap(sht->ctl, old_vx0, old_vy0,
                         old_vx0 + sht->bxsize, old_vy0 + sht->bysize,
                         0);
        sheet_refreshmap(sht->ctl, vx0, vy0,
                         vx0 + sht->bxsize, vy0 + sht->bysize,
                         sht->height);

        sheet_refreshsub(sht->ctl, old_vx0, old_vy0,
                         old_vx0 + sht->bxsize, old_vy0 + sht->bysize,
                         0, sht->height - 1);
        sheet_refreshsub(sht->ctl, vx0, vy0,
                         vx0 + sht->bxsize, vy0 + sht->bysize,
                         sht->height, sht->height);
    }
    return;
}

void sheet_free(struct SHEET *sht)
{
    if (sht->height >= 0) {
        /*  表示中ならまず非表示にする  */
        sheet_updown(sht, -1);
    }
    sht->flags = 0;     /*  未使用マーク。  */
    return;
}
