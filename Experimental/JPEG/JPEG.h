#ifndef __JPEG_H__
#define __JPEG_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define JPEGMetadataFoundStartOfScan 1
#define JPEGMetadataFoundEndOfImage 2
#define JPEGMetadataParsingFailed 3

typedef struct JPEGBlock
{
	int16_t c[64];
	uint8_t eob;
} JPEGBlock;

typedef struct JPEGQuantizationTable
{
	int16_t c[64];
} JPEGQuantizationTable;

typedef struct JPEGHuffmanCode
{
	unsigned int code,length;
} JPEGHuffmanCode;

typedef struct JPEGHuffmanTable
{
	JPEGHuffmanCode codes[256];
} JPEGHuffmanTable;

typedef struct JPEGComponent
{
	unsigned int identifier;
	unsigned int horizontalfactor,verticalfactor;
	JPEGQuantizationTable *quantizationtable;
	unsigned int quantizationindex;
} JPEGComponent;

typedef struct JPEGScanComponent
{
	JPEGComponent *component;
	JPEGHuffmanTable *dctable,*actable;
	unsigned int dcindex,acindex;
} JPEGScanComponent;

typedef struct JPEGMetadata
{
	size_t bytesparsed;

	unsigned int width,height,bits;
	unsigned int restartinterval;

	unsigned int maxhorizontalfactor,maxverticalfactor;
	unsigned int horizontalmcus,verticalmcus;

	unsigned int numcomponents;
	JPEGComponent components[4];

	unsigned int numscancomponents;
	JPEGScanComponent scancomponents[4];

	bool quantizationdefined[4];
	JPEGQuantizationTable quantizationtables[4];

	bool huffmandefined[2][4];
	JPEGHuffmanTable huffmantables[2][4];
} JPEGMetadata;

const void *FindStartOfJPEGImage(const void *bytes,size_t length);

void InitializeJPEGMetadata(JPEGMetadata *self);
int ParseJPEGMetadata(JPEGMetadata *self,const void *bytes,size_t length);

#endif
