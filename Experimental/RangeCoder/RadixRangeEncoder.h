#ifndef __RADIX_RANGE_ENCODER_H__
#define __RADIX_RANGE_ENCODER_H__

#include <stdint.h>
#include <stdbool.h>

extern uint8_t URLSafeAlphabet[71];
extern uint8_t Base64Alphabet[64];
extern uint8_t HexAlphabet[16];

typedef int RadixRangeEncoderWriteFunction(int b,void *writecontext);

#define STDIORadixWriteFunction ((RadixRangeEncoderWriteFunction *)fputc)

typedef struct RadixRangeEncoder
{
	uint32_t radix,bottom,top;

	uint32_t range,low;
	int numextradigits,nextdigit;
	bool overflow,firstdigit;

	RadixRangeEncoderWriteFunction *writefunc;
	void *writecontext;
	bool writefailed;

	uint8_t alphabet[256];
} RadixRangeEncoder;

void InitializeRadixRangeEncoder(RadixRangeEncoder *self,int radix,uint8_t *alphabet,
RadixRangeEncoderWriteFunction *writefunc,void *writecontext);
void FinishRadixRangeEncoder(RadixRangeEncoder *self);
void NormalizeRadixRangeEncoder(RadixRangeEncoder *self);

static inline bool RadixRangeEncoderWritingFailed(RadixRangeEncoder *self) { return self->writefailed; }

static inline int GetRadixRangeEncoderState(RadixRangeEncoder *self,uint32_t *range,uint32_t *low)
{
	*range=self->range;
	*low=self->low;
}

static inline void UpdateRadixRangeEncoderState(RadixRangeEncoder *self,uint32_t newrange,uint32_t newlow)
{
	if((self->top&&newlow>=self->top)||newlow<self->low)
	{
		newlow-=self->top;
		self->overflow=1;
	}

	self->range=newrange;
	self->low=newlow;

	NormalizeRadixRangeEncoder(self);
}

void WriteBitR(RadixRangeEncoder *self,int bit,int weight);
void WriteDynamicBitR(RadixRangeEncoder *self,int bit,int *weight,int shift);

#endif
