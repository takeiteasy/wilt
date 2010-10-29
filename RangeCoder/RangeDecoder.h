#ifndef __RANGE_DECODER_H__
#define __RANGE_DECODER_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct RangeDecoder
{
	uint32_t range,code;

	FILE *fh;
} RangeDecoder;

void InitRangeDecoder(RangeDecoder *self,FILE *fh);
int ReadBit(RangeDecoder *self,int weight);

#endif
