#ifndef __ENCODE_IMPLEMENTATIONS_H__
#define __ENCODE_IMPLEMENTATIONS_H__

#include <stdint.h>
#include <stdbool.h>

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

static void WriteUniversalCodeImplementation(void (*writefunc)(),void *self,
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


#endif
