#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "RangeEncoder.h"
#include "RangeDecoder.h"
#include "RadixRangeEncoder.h"
#include "RadixRangeDecoder.h"

typedef void (*TestFunctionPointer)(int,int,void *);

static void RegularOutput(int bit,int weight,void *context);
static void RegularInput(int bit,int weight,void *context);
static void RadixOutput(int bit,int weight,void *context);
static void RadixInput(int bit,int weight,void *context);

static void RunTests(TestFunctionPointer TestFunction,void *context);
static uint32_t Hash32(uint32_t val);

void Test1()
{
	printf("Running test set 1...\n");

	printf("Creating regular stream \"test1.1.data\"...\n");
	{
		FILE *out=fopen("test1.1.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test1.1.data\"\n");
			exit(1);
		}

		RangeEncoder encoder;
		InitializeRangeEncoder(&encoder,STDIOWriteFunction,out);
		RunTests(RegularOutput,&encoder);
		FinishRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating regular stream \"test1.2.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test1.2.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test1.2.data\"\n");
			exit(1);
		}
	
		RadixRangeEncoder encoder;
		InitializeRadixRangeEncoder(&encoder,256,NULL,STDIOWriteFunction,out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating URL-safe stream \"test1.urlsafe.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test1.urlsafe.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test1.urlsafe.data\"\n");
			exit(1);
		}
	
		RadixRangeEncoder encoder;
		InitializeRadixRangeEncoder(&encoder,sizeof(URLSafeAlphabet),URLSafeAlphabet,STDIOWriteFunction,out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating base64 stream \"test1.base64.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test1.base64.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test1.base64.data\"\n");
			exit(1);
		}
	
		RadixRangeEncoder encoder;
		InitializeRadixRangeEncoder(&encoder,sizeof(Base64Alphabet),Base64Alphabet,STDIOWriteFunction,out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating hex stream \"test1.hex.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test1.hex.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test1.hex.data\"\n");
			exit(1);
		}
	
		RadixRangeEncoder encoder;
		InitializeRadixRangeEncoder(&encoder,sizeof(HexAlphabet),HexAlphabet,STDIOWriteFunction,out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating decimal stream \"test1.decimal.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test1.decimal.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test1.decimal.data\"\n");
			exit(1);
		}
	
		RadixRangeEncoder encoder;
		InitializeRadixRangeEncoder(&encoder,10,(uint8_t *)"0123456789",STDIOWriteFunction,out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating binary stream \"test1.binary.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test1.binary.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test1.binary.data\"\n");
			exit(1);
		}
	
		RadixRangeEncoder encoder;
		InitializeRadixRangeEncoder(&encoder,2,(uint8_t *)"01",STDIOWriteFunction,out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}


	printf("Reading regular stream \"test1.1.data\"...\n");
	{
		FILE *in=fopen("test1.1.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test1.1.data\"\n");
			exit(1);
		}

		RangeDecoder decoder;
		InitializeRangeDecoder(&decoder,STDIOReadFunction,in);
		RunTests(RegularInput,&decoder);
		fclose(in);
	}

	printf("Reading regular stream \"test1.2.data\"...\n");
	{
		FILE *in=fopen("test1.2.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test1.2.data\"\n");
			exit(1);
		}

		RangeDecoder decoder;
		InitializeRangeDecoder(&decoder,STDIOReadFunction,in);
		RunTests(RegularInput,&decoder);
		fclose(in);
	}

	printf("Reading regular stream \"test1.1.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test1.1.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test1.1.data\"\n");
			exit(1);
		}

		RadixRangeDecoder decoder;
		InitializeRadixRangeDecoder(&decoder,256,NULL,STDIOReadFunction,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading regular stream \"test1.2.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test1.2.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test1.2.data\"\n");
			exit(1);
		}

		RadixRangeDecoder decoder;
		InitializeRadixRangeDecoder(&decoder,256,NULL,STDIOReadFunction,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading URL-safe stream \"test1.urlsafe.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test1.urlsafe.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test1.urlsafe.data\"\n");
			exit(1);
		}

		RadixRangeDecoder decoder;
		InitializeRadixRangeDecoder(&decoder,sizeof(URLSafeAlphabet),URLSafeAlphabet,STDIOReadFunction,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading base64 stream \"test1.base64.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test1.base64.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test1.base64.data\"\n");
			exit(1);
		}

		RadixRangeDecoder decoder;
		InitializeRadixRangeDecoder(&decoder,sizeof(Base64Alphabet),Base64Alphabet,STDIOReadFunction,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading hex stream \"test1.hex.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test1.hex.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test1.hex.data\"\n");
			exit(1);
		}

		RadixRangeDecoder decoder;
		InitializeRadixRangeDecoder(&decoder,sizeof(HexAlphabet),HexAlphabet,STDIOReadFunction,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading decimal stream \"test1.decimal.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test1.decimal.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test1.decimal.data\"\n");
			exit(1);
		}

		RadixRangeDecoder decoder;
		InitializeRadixRangeDecoder(&decoder,10,(uint8_t *)"0123456789",STDIOReadFunction,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading binary stream \"test1.binary.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test1.binary.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test1.binary.data\"\n");
			exit(1);
		}

		RadixRangeDecoder decoder;
		InitializeRadixRangeDecoder(&decoder,2,(uint8_t *)"01",STDIOReadFunction,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}
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

static void RadixOutput(int bit,int weight,void *context)
{
	RadixRangeEncoder *encoder=(RadixRangeEncoder *)context;

	WriteBitR(encoder,bit,weight);
}

static void RadixInput(int bit,int weight,void *context)
{
	RadixRangeDecoder *decoder=(RadixRangeDecoder *)context;

	int readbit=ReadBitR(decoder,weight);

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

	for(int i=0;i<0x1000;i++) TestFunction(0,0x800,context);
//	for(int i=0;i<0x1000;i++) TestFunction(1,0x800,context);
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
