#include "RangeDecoder.h"
#include "DecodeImplementations.h"

void InitRangeDecoder(RangeDecoder *self,FILE *fh)
{
	self->range=0xffffffff;
	self->code=0;
	self->fh=fh;

	for(int i=0;i<4;i++) self->code=(self->code<<8)|fgetc(fh);
}

void NormalizeRangeDecoder(RangeDecoder *self)
{
	while(self->range<0x1000000)
	{
		self->code=(self->code<<8)|fgetc(self->fh);
		self->range<<=8;
	}
}

int ReadBit(RangeDecoder *self,int weight)
{
	uint32_t range,code;
	GetRangeDecoderState(self,&range,&code);
	int bit=DecodeBit(&range,&code,weight);
	UpdateRangeDecoderState(self,range,code);
	return bit;
}

int ReadDynamicBit(RangeDecoder *self,int *weight,int shift)
{
	uint32_t range,code;
	GetRangeDecoderState(self,&range,&code);
	int bit=DecodeDynamicBit(&range,&code,weight,shift);
	UpdateRangeDecoderState(self,range,code);
	return bit;
}

uint32_t ReadBitString(RangeDecoder *self,int length,int *weights,int shift)
{
	return ReadBitStringImplementation(ReadDynamicBit,self,length,weights,shift);
}

uint32_t ReadUniversalCode(RangeDecoder *self,
int *weights1,int shift1,int *weights2,int shift2)
{
	return ReadUniversalCodeImplementation(ReadDynamicBit,self,
	weights1,shift1,weights2,shift2);
}

uint32_t ReadUniversalCode2(RangeDecoder *self,
int max,int *weights1,int shift1,int *weights2,int shift2)
{
	return ReadUniversalCode2Implementation(ReadDynamicBit,self,
	max,weights1,shift1,weights2,shift2);
}
