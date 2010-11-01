#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "RangeEncoder.h"
#include "RangeDecoder.h"

typedef void (*TestFunctionPointer)(uint32_t,uint32_t,int *,int,int *,int,void *);

static void Type1Output(uint32_t value,uint32_t max,int *weights1,int shift1,int *weights2,int shift2,void *context);
static void Type1Input(uint32_t value,uint32_t max,int *weights1,int shift1,int *weights2,int shift2,void *context);
static void Type2Output(uint32_t value,uint32_t max,int *weights1,int shift1,int *weights2,int shift2,void *context);
static void Type2Input(uint32_t value,uint32_t max,int *weights1,int shift1,int *weights2,int shift2,void *context);
static void RunTests(TestFunctionPointer TestFunction,void *context);
static uint32_t Hash32(uint32_t val);

void Test5()
{
	printf("Running test set 5...\n");

	printf("Creating stream \"test5.1.data\" using regular universal code...\n");
	{
		FILE *out=fopen("test5.1.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test5.1.data\"\n");
			exit(1);
		}

		RangeEncoder encoder;
		InitRangeEncoder(&encoder,out);
		RunTests(Type1Output,&encoder);
		FinishRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating stream \"test5.2.data\" using bounded universal code...\n");
	{
		FILE *out=fopen("test5.2.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test5.2.data\"\n");
			exit(1);
		}

		RangeEncoder encoder;
		InitRangeEncoder(&encoder,out);
		RunTests(Type2Output,&encoder);
		FinishRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Reading stream \"test5.1.data\"...\n");
	{
		FILE *in=fopen("test5.1.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test5.1.data\"\n");
			exit(1);
		}

		RangeDecoder decoder;
		InitRangeDecoder(&decoder,in);
		RunTests(Type1Input,&decoder);
		fclose(in);
	}

	printf("Reading stream \"test5.2.data\"...\n");
	{
		FILE *in=fopen("test5.2.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test5.2.data\"\n");
			exit(1);
		}

		RangeDecoder decoder;
		InitRangeDecoder(&decoder,in);
		RunTests(Type2Input,&decoder);
		fclose(in);
	}
}

static void Type1Output(uint32_t value,uint32_t max,int *weights1,int shift1,int *weights2,int shift2,void *context)
{
	RangeEncoder *encoder=(RangeEncoder *)context;

	WriteUniversalCode(encoder,value,weights1,shift1,weights2,shift2);
}

static void Type1Input(uint32_t value,uint32_t max,int *weights1,int shift1,int *weights2,int shift2,void *context)
{
	RangeDecoder *decoder=(RangeDecoder *)context;

	uint32_t readvalue=ReadUniversalCode(decoder,weights1,shift1,weights2,shift2);

	assert(value==readvalue);
}

static void Type2Output(uint32_t value,uint32_t max,int *weights1,int shift1,int *weights2,int shift2,void *context)
{
	RangeEncoder *encoder=(RangeEncoder *)context;

	WriteUniversalCode2(encoder,value,max,weights1,shift1,weights2,shift2);
}

static void Type2Input(uint32_t value,uint32_t max,int *weights1,int shift1,int *weights2,int shift2,void *context)
{
	RangeDecoder *decoder=(RangeDecoder *)context;

	uint32_t readvalue=ReadUniversalCode2(decoder,max,weights1,shift1,weights2,shift2);

	assert(value==readvalue);
}

static void RunTests(TestFunctionPointer TestFunction,void *context)
{
	int weights1[33],weights2[33],max;

	for(int i=0;i<33;i++) weights1[i]=weights2[i]=0x800;

	for(int i=0;i<0x1000;i++) TestFunction(0,0,weights1,3,weights2,3,context);

	max=1;
	for(int e=0;e<32;e++)
	{
		for(int i=0;i<0x1000;i++) TestFunction(0,max,weights1,3,weights2,3,context);
		for(int i=0;i<0x1000;i++) TestFunction(max,max,weights1,3,weights2,3,context);
		for(int i=0;i<0x1000;i++) TestFunction(i%(max+1),max,weights1,3,weights2,3,context);
		for(int i=0;i<0x1000;i++)
		{
			uint32_t val=Hash32(i)%(max+1);
			TestFunction(val,max,weights1,3,weights2,3,context);
		}
		max*=2;
	}

	max=1;
	for(int e=0;e<32;e++)
	{
		for(int i=0;i<0x1000;i++) TestFunction(0,max-1,weights1,3,weights2,3,context);
		for(int i=0;i<0x1000;i++) TestFunction(max-1,max-1,weights1,3,weights2,3,context);
		for(int i=0;i<0x1000;i++) TestFunction(i%max,max-1,weights1,3,weights2,3,context);
		for(int i=0;i<0x1000;i++)
		{
			uint32_t val=Hash32(i)%max;
			TestFunction(val,max-1,weights1,3,weights2,3,context);
		}
		max*=2;
	}

	max=1;
	for(int e=0;e<9;e++)
	{
		for(int i=0;i<0x1000;i++) TestFunction(0,max,weights1,3,weights2,3,context);
		for(int i=0;i<0x1000;i++) TestFunction(max,max,weights1,3,weights2,3,context);
		for(int i=0;i<0x1000;i++) TestFunction(i%(max+1),max,weights1,3,weights2,3,context);
		for(int i=0;i<0x1000;i++)
		{
			uint32_t val=Hash32(i)%(max+1);
			TestFunction(val,max,weights1,3,weights2,3,context);
		}
		max*=10;
	}

	for(int i=0;i<0x10000;i++)
	{
		uint32_t max=Hash32(i)>>(Hash32(i+1)%32);
		uint32_t val=Hash32(i+70)%(max+1);
		TestFunction(val,max,weights1,3,weights2,3,context);
	}
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
