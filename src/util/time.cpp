#include "time.h"
#include <cstddef>
long long time_ms()
{
    struct timeval tv;
    long long mstime;
    gettimeofday(&tv, NULL);
    mstime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return mstime;
}