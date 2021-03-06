#ifndef __RANGE_DECODER_H__
#define __RANGE_DECODER_H__

#include <stdint.h>
#include <stdbool.h>

typedef int RangeDecoderReadFunction(void *readcontext);

#define STDIOReadFunction ((RangeDecoderReadFunction *)fgetc)

typedef struct RangeDecoder
{
	uint32_t range,code;

	RangeDecoderReadFunction *readfunc;
	void *readcontext;
	bool eof;
} RangeDecoder;

void InitializeRangeDecoder(RangeDecoder *self,
RangeDecoderReadFunction *readfunc,void *readcontext);
void NormalizeRangeDecoder(RangeDecoder *self);

static inline bool RangeDecoderReachedEOF(RangeDecoder *self) { return self->eof; }

static inline int GetRangeDecoderState(RangeDecoder *self,uint32_t *range,uint32_t *code)
{
	NormalizeRangeDecoder(self);
	*range=self->range;
	*code=self->code;
}

static inline void UpdateRangeDecoderState(RangeDecoder *self,uint32_t newrange,uint32_t newcode)
{
	self->range=newrange;
	self->code=newcode;
}

int ReadBit(RangeDecoder *self,int weight);
int ReadDynamicBit(RangeDecoder *self,int *weight,int shift);
uint32_t ReadBitString(RangeDecoder *self,int length,int *weights,int shift);
uint32_t ReadUniversalCode(RangeDecoder *self,
int *weights1,int shift1,int *weights2,int shift2);
uint32_t ReadUniversalCode2(RangeDecoder *self,
int max,int *weights1,int shift1,int *weights2,int shift2);

#endif
