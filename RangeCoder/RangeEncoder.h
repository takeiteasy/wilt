#ifndef __RANGE_ENCODER_H__
#define __RANGE_ENCODER_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct RangeEncoder
{
	uint32_t range;
	uint64_t low;
	uint8_t cache;
	uint32_t cachesize;

	FILE *fh;
} RangeEncoder;

void InitRangeEncoder(RangeEncoder *self,FILE *fh);
void WriteBit(RangeEncoder *self,int bit,int weight);
void FinishRangeEncoder(RangeEncoder *self);

double CalculateCostOfBit(int bit,int weight);

#endif
