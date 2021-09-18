
#if !defined( OS_INCLUDED_COMMON_STDIO_H )
#    define   OS_INCLUDED_COMMON_STDIO_H

#include <stdarg.h>

int snprintf(char *s, int n, const char *format, ...);
int vsnprintf(char *s, int n, const char *format, va_list arg);

#endif
