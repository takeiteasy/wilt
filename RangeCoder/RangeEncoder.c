#include "RangeEncoder.h"

#include <math.h>

static void NormalizeRangeEncoder(RangeEncoder *self);
static void ShiftOutputFromRangeEncoder(RangeEncoder *self);

void InitRangeEncoder(RangeEncoder *self,FILE *fh)
{
	self->range=0xffffffff;
	self->low=0;
	self->numextrabytes=0;
	self->nextbyte=-1;
	self->overflow=false;

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

		uint32_t oldlow=self->low;
		self->low+=threshold;
		self->overflow|=self->low<oldlow;
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
	int next=(self->low>>24)&0xff;

	if(next==0xff&&self->overflow==0)
	{
		self->numextrabytes++;
	}
	else
	{
		if(self->nextbyte>=0)
		putc((self->nextbyte+self->overflow)&0xff,self->fh);

		for(int i=0;i<self->numextrabytes;i++)
		fputc((0xff+self->overflow)&0xff,self->fh);

		self->numextrabytes=0;
		self->nextbyte=next;
		self->overflow=false;
	}

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
