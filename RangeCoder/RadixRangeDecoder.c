#include "RadixRangeDecoder.h"
#include "DecodeImplementations.h"

#include <string.h>

static int ReadDigit(RadixRangeDecoder *self);

void InitRadixRangeDecoder(RadixRangeDecoder *self,int radix,uint8_t *alphabet,FILE *fh)
{
	int bottom;
	if(radix==2) bottom=0x80000000;
	else if(radix==3) bottom=0x4546b3db;
	else if(radix==4) bottom=0x40000000;
	else if(radix==5) bottom=0xe8d4a51;
	else if(radix==6) bottom=0x159fd800;
	else if(radix==7) bottom=0x10d63af1;
	else
	{
		bottom=radix*radix*radix;
		if(radix<=9) bottom*=radix;
		if(radix<=11) bottom*=radix;
		if(radix<=16) bottom*=radix;
		if(radix<=23) bottom*=radix;
		if(radix<=40) bottom*=radix;
		if(radix<=84) bottom*=radix;
		//if(radix<=256) bottom*=radix;
		//if(radix<=1625) bottom*=radix;
		//if(radix<=65536) bottom*=radix;
	}

	self->radix=radix;
	self->bottom=bottom;

	self->range=bottom*radix-1;
	self->code=0;
	self->fh=fh;

	memset(self->reversealphabet,0,sizeof(self->reversealphabet));
	if(alphabet) for(int i=0;i<radix;i++) self->reversealphabet[alphabet[i]]=i;
	else for(int i=0;i<radix;i++) self->reversealphabet[i]=i;

	int top=bottom*radix;
	for(uint32_t n=1;n!=top;n*=radix)
	self->code=self->code*radix+ReadDigit(self);
}

void NormalizeRadixRangeDecoder(RadixRangeDecoder *self)
{
	while(self->range<self->bottom)
	{
		self->code=self->code*self->radix+ReadDigit(self);
		self->range=self->range*self->radix;
	}
}

static int ReadDigit(RadixRangeDecoder *self)
{
	return self->reversealphabet[fgetc(self->fh)];
}



int ReadBitR(RadixRangeDecoder *self,int weight)
{
	uint32_t range,code;
	GetRadixRangeDecoderState(self,&range,&code);
	int bit=DecodeBit(&range,&code,weight);
	UpdateRadixRangeDecoderState(self,range,code);
	return bit;
}

int ReadDynamicBitR(RadixRangeDecoder *self,int *weight,int shift)
{
	uint32_t range,code;
	GetRadixRangeDecoderState(self,&range,&code);
	int bit=DecodeDynamicBit(&range,&code,weight,shift);
	UpdateRadixRangeDecoderState(self,range,code);
	return bit;
}
