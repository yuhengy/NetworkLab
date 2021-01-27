
#include "endianSwap.h"
#include <endian.h>

void endianSwap(uint8_t *data, int length)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    int cnt,end;
    cnt = length / 2;
    end  = length - 1;
    uint8_t tmp;
    for (int i = 0; i < cnt; i++)
    {
        tmp         = data[i];
        data[i]     = data[end-i];
        data[end-i] = tmp;
    }
#elif __BYTE_ORDER == __BIG_ENDIAN

#endif
}
