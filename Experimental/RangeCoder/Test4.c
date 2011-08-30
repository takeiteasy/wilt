#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "RangeEncoder.h"
#include "RangeDecoder.h"

static uint32_t Hash32(uint32_t val);

void Test4()
{
	printf("Running test set 4...\n");

	printf("Creating stream \"test4.data\" using dynamic bit string coder...\n");

	FILE *out=fopen("test4.data","wb");
	if(!out)
	{
		fprintf(stderr,"Couldn't create file \"test4.data\"\n");
		exit(1);
	}

	RangeEncoder encoder;
	InitializeRangeEncoder(&encoder,STDIOWriteFunction,out);

	int weights[65536];
	for(int length=1;length<=16;length++)
	{
		for(int i=1;i<1<<length;i++) weights[i]=0x800;

		for(int i=0;i<0x1000;i++)
		{
			uint32_t val=Hash32(i)&((1<<length)-1);
			WriteBitString(&encoder,val,length,weights,3);
		}
	}

	FinishRangeEncoder(&encoder);
	fclose(out);

	printf("Reading stream \"test4.data\"...\n");

	FILE *in=fopen("test4.data","rb");
	if(!in)
	{
		fprintf(stderr,"Couldn't read file \"test4.data\"\n");
		exit(1);
	}

	RangeDecoder decoder;
	InitializeRangeDecoder(&decoder,STDIOReadFunction,in);

	for(int length=1;length<=16;length++)
	{
		for(int i=1;i<1<<length;i++) weights[i]=0x800;

		for(int i=0;i<0x1000;i++)
		{
			uint32_t val=Hash32(i)&((1<<length)-1);
			assert(val==ReadBitString(&decoder,length,weights,3));
		}
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
