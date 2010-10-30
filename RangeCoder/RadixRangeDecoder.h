#ifndef __RADIX_RANGE_DECODER_H__
#define __RADIX_RANGE_DECODER_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct RadixRangeDecoder
{
	uint32_t radix,bottom;
	uint32_t range,code;

	FILE *fh;

	uint8_t reversealphabet[256];
} RadixRangeDecoder;

void InitRadixRangeDecoder(RadixRangeDecoder *self,int radix,uint8_t *alphabet,FILE *fh);
int ReadRadixBit(RadixRangeDecoder *self,int weight);

#endif
