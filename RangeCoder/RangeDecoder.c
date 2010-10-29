#include "RangeDecoder.h"

static void NormalizeRangeDecoder(RangeDecoder *self);

void InitRangeDecoder(RangeDecoder *self,FILE *fh)
{
	self->range=0xffffffff;
	self->code=0;
	self->fh=fh;

	for(int i=0;i<4;i++) self->code=(self->code<<8)|fgetc(fh);
}

int ReadBit(RangeDecoder *self,int weight)
{
	NormalizeRangeDecoder(self);

	uint32_t threshold=(self->range>>12)*weight;

	if(self->code<threshold)
	{
		self->range=threshold;
		return 0;
	}
	else
	{
		self->range-=threshold;
		self->code-=threshold;
		return 1;
	}
}

static void NormalizeRangeDecoder(RangeDecoder *self)
{
	while(self->range<0x1000000)
	{
		self->code=(self->code<<8)|fgetc(self->fh);
		self->range<<=8;
	}
}
