#ifndef __CHECKSUMBASE_H__
#define __CHECKSUMBASE_H__

#include <stdint.h>

// calculate the checksum of the given buf, providing sum 
// as the initial value
static inline uint16_t checksumBase(uint16_t *ptr, int nbytes, uint32_t sum)
{
	if (nbytes % 2) {
		sum += ((uint8_t *)ptr)[--nbytes];
	}
 
    while (nbytes > 0) {
        sum += *ptr++;
        nbytes -= 2;
    }
 
    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);

    return (uint16_t)~sum;
}

#endif
