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
void NormalizeRadixRangeDecoder(RadixRangeDecoder *self);

static inline int GetRadixRangeDecoderState(RadixRangeDecoder *self,uint32_t *range,uint32_t *code)
{
	NormalizeRadixRangeDecoder(self);
	*range=self->range;
	*code=self->code;
}

static inline void UpdateRadixRangeDecoderState(RadixRangeDecoder *self,uint32_t newrange,uint32_t newcode)
{
	self->range=newrange;
	self->code=newcode;
}

int ReadBitR(RadixRangeDecoder *self,int weight);
int ReadDynamicBitR(RadixRangeDecoder *self,int *weight,int shift);

#endif
