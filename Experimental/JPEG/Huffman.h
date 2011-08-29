#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct HuffmanTable
{
	struct
	{
		uint8_t value,length;
	} table[65536];
} HuffmanTable;

void InitializeHuffmanTable(HuffmanTable *self);
void AddHuffmanCode(HuffmanTable *self,uint16_t code,unsigned int length,uint8_t value);

typedef struct BitStreamReader
{
	const uint8_t *pos,*end;

	uint32_t bits;
	unsigned int numbits;
} BitStreamReader;

void InitializeBitStreamReader(BitStreamReader *self,const void *bytes,size_t length);
int ReadBitString(BitStreamReader *self,unsigned int length);
int ReadHuffmanCode(BitStreamReader *self,HuffmanTable *table);
bool FlushBitStream(BitStreamReader *self);
bool FlushBitStreamAndSkipRestartMarker(BitStreamReader *self,int n);

#endif

