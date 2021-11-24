
#if !defined( OS_INCLUDED_COMMON_STDLIB_H )
#    define   OS_INCLUDED_COMMON_STDLIB_H

#define RAND_MAX    32768

int rand(void);
int rand_r(unsigned int *seedp);
void srand(unsigned int seed);

#endif
