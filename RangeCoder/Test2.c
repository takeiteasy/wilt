#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "RangeEncoder.h"
#include "RangeDecoder.h"

static uint32_t Hash32(uint32_t val);

void Test2()
{
	printf("Running test set 2...\n");

	printf("Creating stream \"test2.data\" using dynamic bit coder...\n");

	FILE *out=fopen("test2.data","wb");
	if(!out)
	{
		fprintf(stderr,"Couldn't create file \"test2.data\"\n");
		exit(1);
	}

	RangeEncoder encoder;
	InitRangeEncoder(&encoder,out);

	int weight=0x800;
	for(int shift=1;shift<16;shift++)
	{
		for(int i=0;i<0x1000;i++) WriteDynamicBit(&encoder,i&1,&weight,shift);
		for(int i=0;i<0x1000;i++) WriteDynamicBit(&encoder,0,&weight,shift);
		for(int i=0;i<0x1000;i++) WriteDynamicBit(&encoder,1,&weight,shift);
		for(int i=0;i<0x1000;i++) WriteDynamicBit(&encoder,Hash32(i)&1,&weight,shift);
	}

	FinishRangeEncoder(&encoder);
	fclose(out);

	printf("Reading stream \"test2.data\"...\n");

	FILE *in=fopen("test2.data","rb");
	if(!in)
	{
		fprintf(stderr,"Couldn't read file \"test2.data\"\n");
		exit(1);
	}

	RangeDecoder decoder;
	InitRangeDecoder(&decoder,in);

	weight=0x800;
	for(int shift=1;shift<16;shift++)
	{
		for(int i=0;i<0x1000;i++) assert((i&1)==ReadDynamicBit(&decoder,&weight,shift));
		for(int i=0;i<0x1000;i++) assert(0==ReadDynamicBit(&decoder,&weight,shift));
		for(int i=0;i<0x1000;i++) assert(1==ReadDynamicBit(&decoder,&weight,shift));
		for(int i=0;i<0x1000;i++) assert((Hash32(i)&1)==ReadDynamicBit(&decoder,&weight,shift));
	}

	fclose(in);
}

static uint32_t Hash32(uint32_t val)
{
	val^=val>>16;
	val^=61;
	val+=val<<3;
	val^=val>>4;
	val*=0x27d4eb2d;
	val^=val>>15;
	return val;
}
