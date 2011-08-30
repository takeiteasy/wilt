#ifndef __JPEG_DECOMPRESSOR_H__
#define __JPEG_DECOMPRESSOR_H__

//#include "InputStream.h"
#include "JPEG.h"
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
/*	JPEGReadFunction *readfunc;
	void *inputcontext;

	uint32_t metadatalength;
	uint8_t *metadatabytes;

	bool isfirstbundle,reachedend;
	JPEGMetadata jpeg;

	bool slicesavailable;
	unsigned int slicevalue,sliceheight;
	unsigned int currheight,finishedrows;

	int predicted[4];

	uint64_t bitstring;
	unsigned int bitlength;
	bool needsstuffing;

	JPEGArithmeticDecoder decoder;

	JPEGContext eobbins[4][13][63]; // 321 in .
	JPEGContext zerobins[4][62][3][6]; // 1140 in .
	JPEGContext pivotbins[4][63][5][7]; // 2256 in .
	JPEGContext acmagnitudebins[4][3][9][9][9];
	JPEGContext acremainderbins[4][3][7][13];
	JPEGContext acsignbins[4][27][3][2];
	JPEGContext dcmagnitudebins[4][13][10]; // 1 in .
	JPEGContext dcremainderbins[4][13][14]; // 131 in .
	JPEGContext dcsignbins[4][2][2][2]; // 313 in .
	JPEGContext fixedcontext; // 0 in .

	JPEGBlock *blocks[4];

	JPEGBlock *currblock;
	bool mcusavailable;
	unsigned int mcurow,mcucol,mcucomp,mcux,mcuy,mcucoeff;
	unsigned int mcucounter,restartmarkerindex;
	bool writerestartmarker;*/
} JPEGDecompressor;

JPEGDecompressor *AllocJPEGDecompressor(/*JPEGReadFunction *readfunc,void *inputcontext*/);
void FreeJPEGDecompressor(JPEGDecompressor *self);

/*int ReadJPEGHeader(JPEGDecompressor *self);
int ReadNextJPEGBundle(JPEGDecompressor *self);
int ReadNextJPEGSlice(JPEGDecompressor *self);

size_t EncodeJPEGBlocksToBuffer(JPEGDecompressor *self,void *bytes,size_t length);

static inline bool IsFinalJPEGBundle(JPEGDecompressor *self)
{ return self->reachedend; }
static inline bool AreMoreJPEGSlicesAvailable(JPEGDecompressor *self)
{ return !self->reachedend && self->slicesavailable; }
static inline bool AreMoreJPEGBytesAvailable(JPEGDecompressor *self)
{ return self->mcusavailable || self->bitlength>=8 || self->needsstuffing || self->writerestartmarker; }

static inline uint32_t JPEGBundleMetadataLength(JPEGDecompressor *self) { return self->metadatalength; }
static inline uint8_t *JPEGBundleMetadataBytes(JPEGDecompressor *self) { return self->metadatabytes; }*/

#endif

