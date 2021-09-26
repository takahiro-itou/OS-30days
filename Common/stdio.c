
#include "stdio.h"

#define     MAX_WIDTH   32

int int2asc(char *s, int value, int radix, int width, char fill,
            const char *radix_chars)
{
    int len, cnt;
    unsigned int uvalue;
    char buf[MAX_WIDTH];

    if (width >= MAX_WIDTH) {
        width = MAX_WIDTH - 1;
    }
    uvalue = ((value < 0) ? (- value) : (value));
    do {
        buf[len++] = (uvalue % radix);
        uvalue /= radix;
    } while (uvalue);

    cnt = len;
    if (value < 0) {
        *(s ++) = '-';
        ++ cnt;
    }
    while (cnt < width) {
        *(s ++) = fill;
        ++ cnt;
    }

    while (len > 0) {
        int tmp = buf[--len];
        *(s ++) = ((tmp < radix) ? (radix_chars[tmp]) : tmp);
    }
    return ( cnt );
}

int vsnprintf(char *s, int n, const char *format, va_list arg)
{
    const char *p = format;
    int cnt, len, sdata;
    unsigned int udata;
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
                sdata = va_arg(arg, int);
                len = int2asc(buf, sdata, 10, width, fill, "0123456789");
                break;
            case 'x':
                udata = va_arg(arg, unsigned int);
                len = int2asc(buf, udata, 16, width, fill, "0123456789abcdef");
                break;
            case 'X':
                udata = va_arg(arg, unsigned int);
                len = int2asc(buf, udata, 16, width, fill, "0123456789ABCDEF");
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
