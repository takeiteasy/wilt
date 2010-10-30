#ifndef __RADIX_RANGE_ENCODER_H__
#define __RADIX_RANGE_ENCODER_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

extern uint8_t URLSafeAlphabet[71];
extern uint8_t Base64Alphabet[64];
extern uint8_t HexAlphabet[16];

typedef struct RadixRangeEncoder
{
	uint32_t radix,bottom,top;

	uint32_t range,low;
	int numextradigits,nextdigit;
	bool overflow,firstdigit;

	FILE *fh;

	uint8_t alphabet[256];
} RadixRangeEncoder;

void InitRadixRangeEncoder(RadixRangeEncoder *self,int radix,uint8_t *alphabet,FILE *fh);
void WriteRadixBit(RadixRangeEncoder *self,int bit,int weight);
void FinishRadixRangeEncoder(RadixRangeEncoder *self);

#endif
