
#include "stdlib.h"

static unsigned long s_seed = 1;

int myrandom_r(unsigned long *seedp)
{
    unsigned long next = *seedp;
    next = next * 1103515245 + 12345;
    *seedp = next;
    return ( (unsigned)(next / 65536) % RAND_MAX );
}

int rand(void)
{
    return myrandom_r(&s_seed);
}

int rand_r(unsigned int *seedp)
{
    unsigned long seed = *seedp;
    int r = myrandom_r(&seed);
    *seedp = (unsigned int)(seed);
    return r;
}

void srand(unsigned int seed)
{
    s_seed = seed;
}
