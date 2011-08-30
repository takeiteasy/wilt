#ifndef ___JPEG_DECOMPRESSOR_H__
#define ___JPEG_DECOMPRESSOR_H__

#include "JPEG.h"
#include "HuffmanReader.h"
#include "../RangeCoder/RangeEncoder.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

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

	int eobbins[4][13][64];
	int zerobins[4][62][3][6];
	int pivotbins[4][63][5][7];
	int acmagnitudebins[4][3][9][9][13];
	int acremainderbins[4][3][7][13];
	int acsignbins[4][27][3][2];
	int dcmagnitudebins[4][13][14];
	int dcremainderbins[4][13][14];
	int dcsignbins[4][2][2][2];
} JPEGCompressor;

JPEGCompressor *AllocJPEGCompressor(const void *bytes,size_t length);
void FreeJPEGCompressor(JPEGCompressor *self);

void SetJPEGCompressorShifts(JPEGCompressor *self,
int eobshift,int zeroshift,int pivotshift,
int acmagnitudeshift,int acremaindershift,int acsignshift,
int dcmagnitudeshift,int dcremaindershift,int dcsignshift);

bool RunJPEGCompressor(JPEGCompressor *self,FILE *output);

#endif

