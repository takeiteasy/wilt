#include "RangeEncoder.h"
#include "EncodeImplementations.h"

#include <math.h>

static void ShiftOutput(RangeEncoder *self);
static void WriteWrappedDigit(RangeEncoder *self,int output);

void InitRangeEncoder(RangeEncoder *self,FILE *fh)
{
	self->range=0xffffffff;
	self->low=0;
	self->numextrabytes=0;
	self->nextbyte=-1;
	self->overflow=false;

	self->fh=fh;
}

void FinishRangeEncoder(RangeEncoder *self)
{
	for(int i=0;i<5;i++) ShiftOutput(self);
}

void NormalizeRangeEncoder(RangeEncoder *self)
{
	while(self->range<0x1000000)
	{
		self->range<<=8;
		ShiftOutput(self);
	}
}

static void ShiftOutput(RangeEncoder *self)
{
	int next=(self->low>>24)&0xff;

	if(next==0xff&&self->overflow==0)
	{
		self->numextrabytes++;
	}
	else
	{
		if(self->nextbyte>=0)
		WriteWrappedDigit(self,self->nextbyte+self->overflow);

		for(int i=0;i<self->numextrabytes;i++)
		WriteWrappedDigit(self,0xff+self->overflow);

		self->numextrabytes=0;
		self->nextbyte=next;
		self->overflow=false;
	}

	self->low=self->low<<8;
}

static void WriteWrappedDigit(RangeEncoder *self,int output)
{
	fputc(output&0xff,self->fh);
}



void WriteBit(RangeEncoder *self,int bit,int weight)
{
	uint32_t range,low;
	GetRangeEncoderState(self,&range,&low);
	EncodeBit(&range,&low,bit,weight);
	UpdateRangeEncoderState(self,range,low);
}

void WriteDynamicBit(RangeEncoder *self,int bit,int *weight,int shift)
{
	uint32_t range,low;
	GetRangeEncoderState(self,&range,&low);
	EncodeDynamicBit(&range,&low,bit,weight,shift);
	UpdateRangeEncoderState(self,range,low);
}

void WriteBitString(RangeEncoder *self,uint32_t value,int length,int *weights,int shift)
{
	WriteBitStringImplementation(WriteDynamicBit,self,value,length,weights,shift);
}

void WriteUniversalCode(RangeEncoder *self,uint32_t value,
int *weights1,int shift1,int *weights2,int shift2)
{
	WriteUniversalCodeImplementation(WriteDynamicBit,self,value,
	weights1,shift1,weights2,shift2);
}

void WriteUniversalCode2(RangeEncoder *self,uint32_t value,
int max,int *weights1,int shift1,int *weights2,int shift2)
{
	WriteUniversalCode2Implementation(WriteDynamicBit,self,value,
	max,weights1,shift1,weights2,shift2);
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
