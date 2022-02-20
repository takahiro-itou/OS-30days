
#include "string.h"

int memcmp(const void *buf1, const void *buf2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *)buf1;
    const unsigned char *p2 = (const unsigned char *)buf2;
    while ( (n --) > 0 ) {
        if (*p1 != *p2) {
            return (*p1 - *p2);
        }
        ++ p1;
        ++ p2;
    }
    return 0;
}

int strcmp(const char *s1, const char *s2)
{
    while ( *s1 == *s2 ) {
        if (*s1 == '\0') {
            return 0;
        }
        ++ s1;
        ++ s2;
    }
    return ((unsigned char)*s1 - (unsigned char)*s2);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while ( (-- n > 0) && (*s1 == *s2) ) {
        if (*s1 == '\0') {
            return 0;
        }
        ++ s1;
        ++ s2;
    }
    return ((unsigned char)*s1 - (unsigned char)*s2);
}

size_t strlen(const char *s)
{
    size_t  len = 0;
    while (*s++) {
        ++ len;
    }
    return ( len );
}
