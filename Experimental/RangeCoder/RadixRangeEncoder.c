#include "RadixRangeEncoder.h"
#include "EncodeImplementations.h"

#include <string.h>

uint8_t URLSafeAlphabet[71]="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_.!~*'()";
uint8_t Base64Alphabet[64]="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
uint8_t HexAlphabet[16]="0123456789abcdef";

static void Normalize(RadixRangeEncoder *self);
static void ShiftOutput(RadixRangeEncoder *self);
static void WriteWrappedDigit(RadixRangeEncoder *self,int output);

void InitializeRadixRangeEncoder(RadixRangeEncoder *self,int radix,uint8_t *alphabet,
RadixRangeEncoderWriteFunction *writefunc,void *writecontext)
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
	self->top=bottom*radix;

	self->range=bottom*radix-1;
	self->low=0;
	self->numextradigits=0;
	self->nextdigit=-1;
	self->overflow=false;

	self->writefunc=writefunc;
	self->writecontext=writecontext;
	self->writefailed=false;

	if(alphabet) memcpy(self->alphabet,alphabet,radix);
	else for(int i=0;i<radix;i++) self->alphabet[i]=i;
}

void FinishRadixRangeEncoder(RadixRangeEncoder *self)
{
	for(uint32_t n=1;n!=self->top;n*=self->radix) ShiftOutput(self);
	ShiftOutput(self);
}

void NormalizeRadixRangeEncoder(RadixRangeEncoder *self)
{
	while(self->range<self->bottom)
	{
		self->range*=self->radix;
		ShiftOutput(self);
	}
}

static void ShiftOutput(RadixRangeEncoder *self)
{
	int next=self->low/self->bottom;

	if(next==(self->radix-1)&&self->overflow==0)
	{
		self->numextradigits++;
	}
	else
	{
		if(self->nextdigit>=0)
		WriteWrappedDigit(self,self->nextdigit+self->overflow);

		for(int i=0;i<self->numextradigits;i++)
		WriteWrappedDigit(self,self->radix-1+self->overflow);

		self->numextradigits=0;
		self->nextdigit=next;
		self->overflow=false;
	}

	self->low=(self->low%self->bottom)*self->radix;
}

static void WriteWrappedDigit(RadixRangeEncoder *self,int output)
{
	if(self->writefunc(self->alphabet[output%self->radix],self->writecontext)<0)
	self->writefailed=true;
}



void WriteBitR(RadixRangeEncoder *self,int bit,int weight)
{
	uint32_t range,low;
	GetRadixRangeEncoderState(self,&range,&low);
	EncodeBit(&range,&low,bit,weight);
	UpdateRadixRangeEncoderState(self,range,low);
}

void WriteDynamicBitE(RadixRangeEncoder *self,int bit,int *weight,int shift)
{
	uint32_t range,low;
	GetRadixRangeEncoderState(self,&range,&low);
	EncodeDynamicBit(&range,&low,bit,weight,shift);
	UpdateRadixRangeEncoderState(self,range,low);
}

