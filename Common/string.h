
#if !defined( OS_INCLUDED_COMMON_STRING_H )
#    define   OS_INCLUDED_COMMON_STRING_H

typedef unsigned int    size_t;

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

#endif
