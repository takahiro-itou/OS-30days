/*  bootpack  のメイン  */

#include "BootPack.h"
#include "../Common/stdio.h"

void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *)(0xff0);
    char s[40], mcursor[256];
    int mx, my;
    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    snprintf(s, sizeof(s) - 1, "(%d, %d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

    for (;;) {
        io_hlt();
    }
}
