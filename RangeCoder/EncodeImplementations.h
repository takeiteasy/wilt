#ifndef __ENCODE_IMPLEMENTATIONS_H__
#define __ENCODE_IMPLEMENTATIONS_H__

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

static inline void EncodeBit(uint32_t *range,uint32_t *low,int bit,int weight)
{
	uint32_t threshold=(*range>>12)*weight;

	if(bit==0)
	{
		*range=threshold;
	}
	else
	{
		*range-=threshold;
		*low+=threshold;
	}
}

static inline void EncodeDynamicBit(uint32_t *range,uint32_t *low,int bit,int *weight,int shift)
{
	EncodeBit(range,low,bit,*weight);

	if(bit==0) *weight+=(0x1000-*weight)>>shift;
	else *weight-=*weight>>shift;
}

static inline void WriteBitStringImplementation(void (*writefunc)(),void *self,
uint32_t value,int length,int *weights,int shift)
{
	void (*WriteDynamicBitFunc)(void *,int,int *,int)=writefunc;

	for(int i=length-1;i>=0;i--) WriteDynamicBitFunc(self,
	(value>>i)&1,&weights[(value|(1<<length))>>(i+1)],shift);
}

static inline void WriteUniversalCodeImplementation(void (*writefunc)(),void *self,
uint32_t value,int *weights1,int shift1,int *weights2,int shift2)
{
	void (*WriteDynamicBitFunc)(void *,int,int *,int)=writefunc;
	
	int maxbit=31;
	while(maxbit>=0 && (value>>maxbit&1)==0) maxbit--;

	for(int i=0;i<=maxbit;i++) WriteDynamicBitFunc(self,1,&weights1[i],shift1);
	WriteDynamicBitFunc(self,0,&weights1[maxbit+1],shift1);

	for(int i=maxbit-1;i>=0;i--)
	WriteDynamicBitFunc(self,(value>>i)&1,&weights2[i],shift2);
}

static inline void WriteUniversalCode2Implementation(void (*writefunc)(),void *self,
uint32_t value,uint32_t max,int *weights1,int shift1,int *weights2,int shift2)
{
	assert(value<=max);

	void (*WriteDynamicBitFunc)(void *,int,int *,int)=writefunc;

	int maxbit=31;
	while(maxbit>=0 && (value>>maxbit&1)==0) maxbit--;

	for(int i=0;i<=maxbit;i++) WriteDynamicBitFunc(self,1,&weights1[i],shift1);
	if(1<<maxbit+1<=max) WriteDynamicBitFunc(self,0,&weights1[maxbit+1],shift1);

	for(int i=maxbit-1;i>=0;i--)
	{
		uint32_t higher=((value>>i)|1)<<i;

		if(higher<=max)
		WriteDynamicBitFunc(self,(value>>i)&1,&weights2[i],shift2);
	}
}


#endif
