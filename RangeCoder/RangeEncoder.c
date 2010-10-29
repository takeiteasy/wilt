#include "RangeEncoder.h"

#include <math.h>

static void NormalizeRangeEncoder(RangeEncoder *self);
static void ShiftOutputFromRangeEncoder(RangeEncoder *self);

void InitRangeEncoder(RangeEncoder *self,FILE *fh)
{
	self->range=0xffffffff;
	self->low=0;
	self->cache=0xff;
	self->cachesize=0;

	self->fh=fh;
}

void WriteBit(RangeEncoder *self,int bit,int weight)
{
	uint32_t threshold=(self->range>>12)*weight;

	if(bit==0)
	{
		self->range=threshold;
	}
	else
	{
		self->range-=threshold;
		self->low+=threshold;
	}

	NormalizeRangeEncoder(self);
}

void FinishRangeEncoder(RangeEncoder *self)
{
	for(int i=0;i<5;i++)
	{
		ShiftOutputFromRangeEncoder(self);
	}
}

static void NormalizeRangeEncoder(RangeEncoder *self)
{
	while(self->range<0x1000000)
	{
		self->range<<=8;
		ShiftOutputFromRangeEncoder(self);
	}
}

static void ShiftOutputFromRangeEncoder(RangeEncoder *self)
{
	if((self->low&0xffffffff)<0xff000000||(self->low>>32)!=0)
	{
		uint8_t temp=self->cache;
		for(int i=0;i<self->cachesize;i++)
		{
			fputc((temp+(self->low>>32))&0xff,self->fh);
			temp=0xff;
		}
		self->cachesize=0;
		self->cache=(self->low>>24)&0xff;
	}
	self->cachesize++;
	self->low=(self->low<<8)&0xffffffff;
}




double CalculateCostOfBit(int bit,int weight)
{
	double cost;
	if(bit==0)
	{
		cost=log2((double)0x1000/(double)weight);
	}
	else
	{
		cost=log2((double)0x1000/(double)(0x1000-weight));
	}
	return cost;
}
