#ifndef __HUFFMAN_WRITER_H__
#define __HUFFMAN_WRITER_H__

#include "JPEG.h"
#include "Primitives.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct BitStreamWriter
{
	uint64_t bits;
	unsigned int numbits;

	bool needsstuffing;

	bool needsrestartmarker;
	int restartmarkerindex;
} BitStreamWriter;

static void InitializeBitStreamWriter(BitStreamWriter *self)
{
	self->bits=0;
	self->numbits=0;

	self->needsstuffing=false;
}

static void PushBitString(BitStreamWriter *self,uint32_t bitstring,unsigned int length)
{
	self->bits|=(uint64_t)bitstring<<64-self->numbits-length;
	self->numbits+=length;
}

static void PushHuffmanCode(BitStreamWriter *self,JPEGHuffmanTable *table,unsigned int code)
{
	PushBitString(self,table->codes[code].code,table->codes[code].length);
}

static void PushEncodedValue(BitStreamWriter *self,JPEGHuffmanTable *table,
int value,unsigned int highbits)
{
	int category,bitstring;
	if(value>=0)
	{
		category=Category(value);
		int mask=(1<<category)-1;
		bitstring=value&mask;
	}
	else
	{
		category=Category(-value);
		int mask=(1<<category)-1;
		bitstring=(value&mask)-1;
	}

	PushHuffmanCode(self,table,category|(highbits<<4));
	PushBitString(self,bitstring,category);
}

static void FlushBitStream(BitStreamWriter *self)
{
	// Pad with ones up to a byte boundary.
	int n=(-self->numbits)&7;
	PushBitString(self,(1<<n)-1,n);
}

static void FlushAndPushRestartMarker(BitStreamWriter *self,int restartmarkerindex)
{
	FlushBitStream(self);
	self->restartmarkerindex=restartmarkerindex;
	self->needsrestartmarker=true;
}

static bool ByteAvailableFromBitStream(BitStreamWriter *self)
{
	return self->numbits>=8 || self->needsstuffing || self->needsrestartmarker;
}

static int OutputByteFromBitStream(BitStreamWriter *self)
{
	if(self->needsstuffing)
	{
		// If we need to add a byte of stuffing, output that.
		self->needsstuffing=false;

		return 0x00;
	}
	else if(self->numbits>=8)
	{
		// If there are enough buffered bits, output one byte.
		uint8_t byte=self->bits>>56LL;
		self->bits<<=8;
		self->numbits-=8;

		if(byte==0xff) self->needsstuffing=true;
		return byte;
	}
	else if(self->needsrestartmarker)
	{
		// If we need to add a restart marker, output that.
		self->needsrestartmarker=false;

		// Push the second byte of the marker into the bitstream,
		// and output the first.
		PushBitString(self,0xd0+self->restartmarkerindex,8);
		return 0xff;
	}
}

#endif
