#include <stdio.h>
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

int main(int argc,char **argv)
{
	printf("Creating regular stream \"test.1.data\"...\n");
	{
		FILE *out=fopen("test.1.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test.1.data\"\n");
			return 1;
		}

		RangeEncoder encoder;
		InitRangeEncoder(&encoder,out);
		RunTests(RegularOutput,&encoder);
		FinishRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating regular stream \"test.2.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test.2.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test.2.data\"\n");
			return 1;
		}
	
		RadixRangeEncoder encoder;
		InitRadixRangeEncoder(&encoder,256,NULL,out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating URL-safe stream \"test.urlsafe.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test.urlsafe.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test.urlsafe.data\"\n");
			return 1;
		}
	
		RadixRangeEncoder encoder;
		InitRadixRangeEncoder(&encoder,sizeof(URLSafeAlphabet),URLSafeAlphabet,out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating base64 stream \"test.base64.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test.base64.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test.base64.data\"\n");
			return 1;
		}
	
		RadixRangeEncoder encoder;
		InitRadixRangeEncoder(&encoder,sizeof(Base64Alphabet),Base64Alphabet,out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating hex stream \"test.hex.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test.hex.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test.hex.data\"\n");
			return 1;
		}
	
		RadixRangeEncoder encoder;
		InitRadixRangeEncoder(&encoder,sizeof(HexAlphabet),HexAlphabet,out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating decimal stream \"test.decimal.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test.decimal.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test.decimal.data\"\n");
			return 1;
		}
	
		RadixRangeEncoder encoder;
		InitRadixRangeEncoder(&encoder,10,(uint8_t *)"0123456789",out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}

	printf("Creating binary stream \"test.binary.data\" using radix encoder...\n");
	{
		FILE *out=fopen("test.binary.data","wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't create file \"test.binary.data\"\n");
			return 1;
		}
	
		RadixRangeEncoder encoder;
		InitRadixRangeEncoder(&encoder,2,(uint8_t *)"01",out);
		RunTests(RadixOutput,&encoder);
		FinishRadixRangeEncoder(&encoder);
		fclose(out);
	}


	printf("Reading regular stream \"test.1.data\"...\n");
	{
		FILE *in=fopen("test.1.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test.1.data\"\n");
			return 1;
		}

		RangeDecoder decoder;
		InitRangeDecoder(&decoder,in);
		RunTests(RegularInput,&decoder);
		fclose(in);
	}

	printf("Reading regular stream \"test.2.data\"...\n");
	{
		FILE *in=fopen("test.2.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test.2.data\"\n");
			return 1;
		}

		RangeDecoder decoder;
		InitRangeDecoder(&decoder,in);
		RunTests(RegularInput,&decoder);
		fclose(in);
	}

	printf("Reading regular stream \"test.1.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test.1.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test.1.data\"\n");
			return 1;
		}

		RadixRangeDecoder decoder;
		InitRadixRangeDecoder(&decoder,256,NULL,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading regular stream \"test.2.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test.2.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test.2.data\"\n");
			return 1;
		}

		RadixRangeDecoder decoder;
		InitRadixRangeDecoder(&decoder,256,NULL,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading URL-safe stream \"test.urlsafe.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test.urlsafe.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test.urlsafe.data\"\n");
			return 1;
		}

		RadixRangeDecoder decoder;
		InitRadixRangeDecoder(&decoder,sizeof(URLSafeAlphabet),URLSafeAlphabet,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading base64 stream \"test.base64.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test.base64.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test.base64.data\"\n");
			return 1;
		}

		RadixRangeDecoder decoder;
		InitRadixRangeDecoder(&decoder,sizeof(Base64Alphabet),Base64Alphabet,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading hex stream \"test.hex.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test.hex.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test.hex.data\"\n");
			return 1;
		}

		RadixRangeDecoder decoder;
		InitRadixRangeDecoder(&decoder,sizeof(HexAlphabet),HexAlphabet,in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading decimal stream \"test.decimal.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test.decimal.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test.decimal.data\"\n");
			return 1;
		}

		RadixRangeDecoder decoder;
		InitRadixRangeDecoder(&decoder,10,(uint8_t *)"0123456789",in);
		RunTests(RadixInput,&decoder);
		fclose(in);
	}

	printf("Reading binary stream \"test.binary.data\" using radix decoder...\n");
	{
		FILE *in=fopen("test.binary.data","rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't read file \"test.binary.data\"\n");
			return 1;
		}

		RadixRangeDecoder decoder;
		InitRadixRangeDecoder(&decoder,2,(uint8_t *)"01",in);
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

	WriteRadixBit(encoder,bit,weight);
}

static void RadixInput(int bit,int weight,void *context)
{
	RadixRangeDecoder *decoder=(RadixRangeDecoder *)context;

	int readbit=ReadRadixBit(decoder,weight);

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
