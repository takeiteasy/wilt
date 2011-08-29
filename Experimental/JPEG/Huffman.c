#include "Huffman.h"

#include <string.h>

static void FillBits(BitStreamReader *self,unsigned int required);
static void DiscardBits(BitStreamReader *self,unsigned int length);

void InitializeHuffmanTable(HuffmanTable *self)
{
	memset(self->table,0,sizeof(self->table));
}

void AddHuffmanCode(HuffmanTable *self,uint16_t code,unsigned int length,uint8_t value)
{
	for(int i=0;i<1<<16-length;i++)
	{
		self->table[(code<<16-length)+i].value=value;
		self->table[(code<<16-length)+i].length=length;
	}
}

void InitializeBitStreamReader(BitStreamReader *self,const void *bytes,size_t length)
{
	self->pos=bytes;
	self->end=self->pos+length;
	self->bits=0;
	self->numbits=0;
}

int ReadBitString(BitStreamReader *self,unsigned int length)
{
	FillBits(self,length);
	if(self->numbits<length) return -1;

	uint32_t bitstring=self->bits>>32-length;

	DiscardBits(self,length);

	return bitstring;
}

int ReadHuffmanCode(BitStreamReader *self,HuffmanTable *table)
{
	FillBits(self,16);

	int index=self->bits>>16;
	uint8_t value=table->table[index].value;
	unsigned int length=table->table[index].length;

	if(self->numbits<length) return -1;
	if(length==0) return -2;

	DiscardBits(self,length);

	return value;
}

void FlushBitStream(BitStreamReader *self)
{
	DiscardBits(self,self->numbits&7);
}

static void FillBits(BitStreamReader *self,unsigned int required)
{
	while(self->numbits<required)
	{
		if(self->pos>=self->end) return;
		self->bits|=(*self->pos++)<<(24-self->numbits);
		self->numbits+=8;
	}
}

static void DiscardBits(BitStreamReader *self,unsigned int length)
{
	self->bits<<=length;
	self->numbits-=length;
}

