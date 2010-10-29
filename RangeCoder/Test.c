#include <stdio.h>
#include <assert.h>

#include "RangeEncoder.h"
#include "RangeDecoder.h"

typedef void (*TestFunctionPointer)(int,int,void *);

static void RegularOutput(int bit,int weight,void *context);
static void RegularInput(int bit,int weight,void *context);

static void RunTests(TestFunctionPointer TestFunction,void *context);
static uint32_t Hash32(uint32_t val);

int main(int argc,char **argv)
{
	FILE *out=fopen("test1.data","wb");
	if(!out)
	{
		fprintf(stderr,"Couldn't create file \"test1.data\"\n");
		return 1;
	}

	RangeEncoder encoder;
	InitRangeEncoder(&encoder,out);
	RunTests(RegularOutput,&encoder);
	FinishRangeEncoder(&encoder);
	fclose(out);

	FILE *in=fopen("test1.data","rb");
	if(!in)
	{
		fprintf(stderr,"Couldn't read file \"test1.data\"\n");
		return 1;
	}

	RangeDecoder decoder;
	InitRangeDecoder(&decoder,in);
	RunTests(RegularInput,&decoder);
	fclose(in);
}



static void RegularOutput(int bit,int weight,void *context)
{
	RangeEncoder *encoder=(RangeEncoder *)context;

	WriteBit(encoder,bit,weight);
}

static void RegularInput(int bit,int weight,void *context)
{
	RangeDecoder *decoder=(RangeDecoder *)context;

	int readbit=ReadBit(decoder,weight);

	assert(bit==readbit);
}




static void RunTests(TestFunctionPointer TestFunction,void *context)
{
	for(int i=0;i<0x1000;i++) TestFunction(i&1,0x800,context);
	for(int i=0;i<0x1000;i++) TestFunction(0,0x800,context);
	for(int i=0;i<0x1000;i++) TestFunction(1,0x800,context);

	for(int i=0;i<0x1000;i++) TestFunction(i&1,0x001,context);
	for(int i=0;i<0x1000;i++) TestFunction(0,0x001,context);
	for(int i=0;i<0x1000;i++) TestFunction(1,0x001,context);

	for(int i=0;i<0x1000;i++) TestFunction(i&1,0xfff,context);
	for(int i=0;i<0x1000;i++) TestFunction(0,0xfff,context);
	for(int i=0;i<0x1000;i++) TestFunction(1,0xfff,context);

	for(int i=1;i<=0xfff;i++) TestFunction(i&1,i,context);
	for(int i=1;i<=0xfff;i++) TestFunction(0,i,context);
	for(int i=1;i<=0xfff;i++) TestFunction(1,i,context);

	for(int i=0;i<0x1000;i++) TestFunction(i&1,1+Hash32(i)%0xffe,context);
	for(int i=0;i<0x1000;i++) TestFunction(0,1+Hash32(i)%0xffe,context);
	for(int i=0;i<0x1000;i++) TestFunction(1,1+Hash32(i)%0xffe,context);
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
