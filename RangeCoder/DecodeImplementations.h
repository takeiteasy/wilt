#ifndef __DECODE_IMPLEMENTATIONS_H__
#define __DECODE_IMPLEMENTATIONS_H__

#include <stdint.h>
#include <stdbool.h>

static inline int DecodeBit(uint32_t *range,uint32_t *code,int weight)
{
	uint32_t threshold=(*range>>12)*weight;

	if(*code<threshold)
	{
		*range=threshold;
		return 0;
	}
	else
	{
		*range-=threshold;
		*code-=threshold;
		return 1;
	}
}

static inline int DecodeDynamicBit(uint32_t *range,uint32_t *low,int *weight,int shift)
{
	int bit=DecodeBit(range,low,*weight);

	if(bit==0) *weight+=(0x1000-*weight)>>shift;
	else *weight-=*weight>>shift;

	return bit;
}

static uint32_t ReadUniversalCodeImplementation(int (*readfunc)(),void *self,
int *weights1,int shift1,int *weights2,int shift2)
{
	int (*ReadDynamicBitFunc)(void *,int *,int)=readfunc;
	int numbits=0;

	while(ReadDynamicBitFunc(self,&weights1[numbits],shift1)==1) numbits++;
	if(!numbits) return 0;

	uint32_t val=1;

	for(int i=0;i<numbits-1;i++)
	val=(val<<1)|ReadDynamicBitFunc(self,&weights2[numbits-1-i],shift2);

	return val;
}

#endif
