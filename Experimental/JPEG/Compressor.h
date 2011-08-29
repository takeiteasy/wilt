#ifndef ___JPEG_DECOMPRESSOR_H__
#define ___JPEG_DECOMPRESSOR_H__

#include "JPEG.h"
#include "Huffman.h"
#include "../RangeCoder/RangeEncoder.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct JPEGCompressor
{
	const uint8_t *start,*end;

	JPEGMetadata jpeg;
	BitStreamReader bitstream;
	HuffmanTable dctables[4];
	HuffmanTable actables[4];

	int predicted[4];

	RangeEncoder encoder;

	int eobshift,zeroshift,pivotshift;
	int acmagnitudeshift,acremaindershift,acsignshift;
	int dcmagnitudeshift,dcremaindershift,dcsignshift;

	uint16_t eobbins[4][13][63];
	uint16_t zerobins[4][62][3][6];
	uint16_t pivotbins[4][63][5][7];
	uint16_t acmagnitudebins[4][3][9][9][9];
	uint16_t acremainderbins[4][3][7][13];
	uint16_t acsignbins[4][27][3][2];
	uint16_t dcmagnitudebins[4][13][10];
	uint16_t dcremainderbins[4][13][14];
	uint16_t dcsignbins[4][2][2][2];
} JPEGCompressor;

JPEGCompressor *AllocJPEGCompressor(const void *bytes,size_t length);
void FreeJPEGCompressor(JPEGCompressor *self);

bool TestJPEGCompressor(JPEGCompressor *self,FILE *output);

#endif

