#ifndef __JPEG_DECOMPRESSOR_H__
#define __JPEG_DECOMPRESSOR_H__

#include "JPEG.h"
#include "HuffmanWriter.h"
#include "../RangeCoder/RangeDecoder.h"

#include <stdint.h>
#include <stdbool.h>

#define JPEGNoError 0
#define JPEGEndOfStreamError 1
#define JPEGOutOfMemoryError 2
#define JPEGInvalidHeaderError 3
#define JPEGLZMAError 4
#define JPEGParseError 5

typedef struct JPEGDecompressor
{
	RangeDecoder decoder;

	uint32_t metadatalength;
	uint8_t *metadatabytes;

	bool isfirstbundle,reachedend;
	JPEGMetadata jpeg;

	int predicted[4];

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

	JPEGBlock *rows[4][3];

	BitStreamWriter bitstream;

	JPEGBlock *currblock;
	bool mcusavailable,neednewmcu;
	unsigned int mcurow,mcucol,mcucomp,mcux,mcuy,mcucoeff;
	unsigned int mcucounter,restartmarkerindex;
} JPEGDecompressor;

JPEGDecompressor *AllocJPEGDecompressor(RangeDecoderReadFunction *readfunc,void *readcontext);
void FreeJPEGDecompressor(JPEGDecompressor *self);

int ReadJPEGHeader(JPEGDecompressor *self);
int ReadNextJPEGBundle(JPEGDecompressor *self);

size_t EncodeJPEGBlocksToBuffer(JPEGDecompressor *self,void *bytes,size_t length);

static inline bool IsFinalJPEGBundle(JPEGDecompressor *self)
{ return self->reachedend; }
static inline bool AreMoreJPEGBytesAvailable(JPEGDecompressor *self)
{ return self->mcusavailable || ByteAvailableFromBitStream(&self->bitstream); }

static inline uint32_t JPEGBundleMetadataLength(JPEGDecompressor *self) { return self->metadatalength; }
static inline uint8_t *JPEGBundleMetadataBytes(JPEGDecompressor *self) { return self->metadatabytes; }

#endif

