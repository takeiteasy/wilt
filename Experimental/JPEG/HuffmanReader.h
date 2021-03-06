#ifndef __HUFFMAN_READER_H__
#define __HUFFMAN_READER_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>




typedef struct HuffmanTable
{
	struct
	{
		uint8_t value,length;
	} table[65536];
} HuffmanTable;

static void InitializeHuffmanTable(HuffmanTable *self)
{
	memset(self->table,0,sizeof(self->table));
}

static void AddHuffmanCode(HuffmanTable *self,uint16_t code,unsigned int length,uint8_t value)
{
	for(int i=0;i<1<<16-length;i++)
	{
		self->table[(code<<16-length)+i].value=value;
		self->table[(code<<16-length)+i].length=length;
	}
}




typedef struct BitStreamReader
{
	const uint8_t *pos,*end;

	uint32_t bits;
	unsigned int numbits;
} BitStreamReader;

static void FillBits(BitStreamReader *self,unsigned int required);
static void DiscardBits(BitStreamReader *self,unsigned int length);

static void InitializeBitStreamReader(BitStreamReader *self,const void *bytes,size_t length)
{
	self->pos=bytes;
	self->end=self->pos+length;
	self->bits=0;
	self->numbits=0;
}

static int ReadBitString(BitStreamReader *self,unsigned int length)
{
	if(length==0) return 0;

	FillBits(self,length);
	if(self->numbits<length) return -1;

	uint32_t bitstring=self->bits>>32-length;

	DiscardBits(self,length);

	return bitstring;
}

static int ReadHuffmanCode(BitStreamReader *self,HuffmanTable *table)
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

static bool FlushBitStream(BitStreamReader *self)
{
	int extrabits=self->numbits&7;
	if(extrabits==0) return true;

	return ReadBitString(self,extrabits)==(1<<extrabits)-1;
}

static bool FlushAndSkipRestartMarker(BitStreamReader *self,int restartmarkerindex)
{
	if(!FlushBitStream(self)) return false;
	if(self->pos+2>self->end) return false;
	if(self->pos[0]!=0xff) return false;	
	if(self->pos[1]!=0xd0+restartmarkerindex) return false;	
	self->pos+=2;
	return true;
}

static void FillBits(BitStreamReader *self,unsigned int required)
{
	while(self->numbits<required)
	{
		if(self->pos>=self->end) return;
		int b=self->pos[0];

		if(b==0xff)
		{
			if(self->pos+1>=self->end) return;
			int b2=self->pos[1];
			if(b2!=0) return;
			self->pos+=2;
		}
		else self->pos++;

		self->bits|=b<<(24-self->numbits);
		self->numbits+=8;
	}
}

static void DiscardBits(BitStreamReader *self,unsigned int length)
{
	self->bits<<=length;
	self->numbits-=length;
}

#endif
