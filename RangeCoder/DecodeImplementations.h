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

static inline uint32_t ReadBitStringImplementation(int (*readfunc)(),void *self,
int length,int *weights,int shift)
{
	int (*ReadDynamicBitFunc)(void *,int *,int)=readfunc;

	uint32_t value=1;
 	for(int i=0;i<length;i++)
 	{
		int bit=ReadDynamicBitFunc(self,&weights[value],shift);
		value=(value<<1)|bit;
	}
	return value^(1<<length);
}

static inline uint32_t ReadUniversalCodeImplementation(int (*readfunc)(),void *self,
int *weights1,int shift1,int *weights2,int shift2)
{
	int (*ReadDynamicBitFunc)(void *,int *,int)=readfunc;

	if(ReadDynamicBitFunc(self,&weights1[0],shift1)==0) return 0;

	int maxbit=0;
	while(ReadDynamicBitFunc(self,&weights1[maxbit+1],shift1)==1) maxbit++;

	uint32_t value=1<<maxbit;
	for(int i=maxbit-1;i>=0;i--)
	{
		uint32_t higher=value|(1<<i);
		if(ReadDynamicBitFunc(self,&weights2[i],shift2))
		value=higher;
	}

	return value;
}

static inline uint32_t ReadUniversalCode2Implementation(int (*readfunc)(),void *self,
uint32_t max,int *weights1,int shift1,int *weights2,int shift2)
{
	int (*ReadDynamicBitFunc)(void *,int *,int)=readfunc;

	if(max==0) return 0;
	if(ReadDynamicBitFunc(self,&weights1[0],shift1)==0) return 0;

	int maxbit=0;
	while(1<<maxbit+1<=max &&
	ReadDynamicBitFunc(self,&weights1[maxbit+1],shift1)==1) maxbit++;

	uint32_t value=1<<maxbit;
	for(int i=maxbit-1;i>=0;i--)
	{
		uint32_t higher=value|(1<<i);
		if(higher<=max && ReadDynamicBitFunc(self,&weights2[i],shift2))
		value=higher;
	}

	return value;
}

#endif
