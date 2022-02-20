
#include "apilib.h"

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

void HariMain(void)
{
}