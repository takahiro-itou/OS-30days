
#include "stdio.h"

#define     MAX_WIDTH   32

int int2asc(char *s, int value, int radix, int width, char fill) {
    int len, cnt;
    char buf[MAX_WIDTH];

    if (width >= MAX_WIDTH) {
        width = MAX_WIDTH;
    }
    while (value) {
        buf[len++] = (value % radix);
        value /= radix;
    }

    cnt = len;
    while (cnt < width) {
        *(s ++) = fill;
        ++ cnt;
    }

    while (len > 0) {
        int tmp = buf[--len];
        *(s ++) = ((tmp < 10) ? (tmp + '0') : (tmp + 0x37));
    }
    return ( cnt );
}

int vsnprintf(char *s, int n, const char *format, va_list arg)
{
    const char *p = format;
    int cnt, len;
    char buf[32];

    for (cnt = 0; (cnt < n); ++ p) {
        const char ch = (*p);
        if (ch == '\0') { break; }
        if (ch == '%') {
            int width = 0;
            char fill = ' ';
            char fmt;
            ++ p;
            if ((*p) == '0') {
                fill = '0';
                ++ p;
            }
            fmt = (*p);
            while ( ('0' <= fmt) && (fmt <= '9') ) {
                width = (width * 10) + (fmt - '0');
                fmt = *(++ p);
            }
            switch (fmt) {
            case 'd':
                len = int2asc(buf, va_arg(arg, int), 10, width, fill);
                break;
            case 'x':
                len = int2asc(buf, va_arg(arg, int), 16, width, fill);
                break;
            }
            for (int i = 0; (i < len) && (cnt < n); ++ cnt, ++ i ) {
                s[cnt]  = buf[i];
            }
        } else {
            s[cnt]  = ch;
            ++ cnt;
        }
    }

    if (n <= cnt) {
        cnt = n - 1;
    }
    s[cnt]  = '\0';
    return ( cnt );
}

int snprintf(char *s, int n, const char *format, ...)
{
    va_list arg;
    int len;

    va_start(arg, format);
    len = vsnprintf(s, n, format, arg);
    va_end(arg);
    return ( len );
}
