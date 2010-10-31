#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "RangeEncoder.h"
#include "RangeDecoder.h"

static uint32_t Hash32(uint32_t val);

void Test3()
{
	printf("Running test set 3...\n");

	printf("Creating regular stream \"test3.data\"...\n");

	FILE *out=fopen("test3.data","wb");
	if(!out)
	{
		fprintf(stderr,"Couldn't create file \"test3.data\"\n");
		exit(1);
	}

	RangeEncoder encoder;
	InitRangeEncoder(&encoder,out);

	int weights1[33],weights2[33];

	for(int i=0;i<33;i++) weights1[i]=weights2[i]=0x800;

	for(int i=0;i<0x1000;i++) WriteUniversalCode(&encoder,0,weights1,3,weights2,3);
	for(int i=0;i<0x1000;i++) WriteUniversalCode(&encoder,0xffffffff,weights1,3,weights2,3);
	for(int i=0;i<0x1000;i++) WriteUniversalCode(&encoder,i,weights1,3,weights2,3);
	for(int i=0;i<0x10000;i++)
	{
		uint32_t val=Hash32(i)>>(Hash32(i+1)%32);
		WriteUniversalCode(&encoder,val,weights1,3,weights2,3);
	}

	FinishRangeEncoder(&encoder);
	fclose(out);

	printf("Reading regular stream \"test3.data\"...\n");

	FILE *in=fopen("test3.data","rb");
	if(!in)
	{
		fprintf(stderr,"Couldn't read file \"test3.data\"\n");
		exit(1);
	}

	RangeDecoder decoder;
	InitRangeDecoder(&decoder,in);

	for(int i=0;i<33;i++) weights1[i]=weights2[i]=0x800;

	for(int i=0;i<0x1000;i++) assert(0==ReadUniversalCode(&decoder,weights1,3,weights2,3));
	for(int i=0;i<0x1000;i++) assert(0xffffffff==ReadUniversalCode(&decoder,weights1,3,weights2,3));
	for(int i=0;i<0x1000;i++) assert(i==ReadUniversalCode(&decoder,weights1,3,weights2,3));
	for(int i=0;i<0x10000;i++)
	{
		uint32_t val=Hash32(i)>>(Hash32(i+1)%32);
		assert(val==ReadUniversalCode(&decoder,weights1,3,weights2,3));
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
