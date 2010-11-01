#ifndef __RANGE_ENCODER_H__
#define __RANGE_ENCODER_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct RangeEncoder
{
	uint32_t range,low;
	int numextrabytes,nextbyte;
	bool overflow,firstbyte;

	FILE *fh;
} RangeEncoder;

void InitRangeEncoder(RangeEncoder *self,FILE *fh);
void FinishRangeEncoder(RangeEncoder *self);
void NormalizeRangeEncoder(RangeEncoder *self);

static inline int GetRangeEncoderState(RangeEncoder *self,uint32_t *range,uint32_t *low)
{
	*range=self->range;
	*low=self->low;
}

static inline void UpdateRangeEncoderState(RangeEncoder *self,uint32_t newrange,uint32_t newlow)
{
	self->overflow|=newlow<self->low;
	self->range=newrange;
	self->low=newlow;

	NormalizeRangeEncoder(self);
}

void WriteBit(RangeEncoder *self,int bit,int weight);
void WriteDynamicBit(RangeEncoder *self,int bit,int *weight,int shift);
void WriteBitString(RangeEncoder *self,uint32_t value,int length,int *weights,int shift);
void WriteUniversalCode(RangeEncoder *self,uint32_t value,
int *weights1,int shift1,int *weights2,int shift2);
void WriteUniversalCode2(RangeEncoder *self,uint32_t value,
int max,int *weights1,int shift1,int *weights2,int shift2);

double CalculateCostOfBit(int bit,int weight);

#endif
