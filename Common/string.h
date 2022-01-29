
#if !defined( OS_INCLUDED_COMMON_STRING_H )
#    define   OS_INCLUDED_COMMON_STRING_H

typedef unsigned int    size_t;

int memcmp(const void *buf1, const void *buf2, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

#endif
