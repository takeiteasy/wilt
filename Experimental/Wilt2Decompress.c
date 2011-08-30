#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "RangeCoder/RangeDecoder.h"




static void CopyMemory(uint8_t *dest,uint8_t *src,int length)
{
	for(int i=0;i<length;i++) *dest++=*src++;
}

void DecompressData(FILE *fh,uint8_t *buf,uint32_t size,
int typeshift,int literalshift,int lengthshift1,int lengthshift2,int offsetshift1,int offsetshift2)
{
	RangeDecoder dec;
	InitializeRangeDecoder(&dec,STDIOReadFunction,fh);

	int typeweight=0x800;

	int lengthweights1[32],lengthweights2[32];
	int offsetweights1[32],offsetweights2[32];
	for(int i=0;i<32;i++)
	lengthweights1[i]=lengthweights2[i]=offsetweights1[i]=offsetweights2[i]=0x800;

	int literalbitweights[256];
	for(int i=0;i<256;i++)
	literalbitweights[i]=0x800;

	int pos=0;
	while(pos<size)
	{
		int length,offs;

		if(ReadDynamicBit(&dec,&typeweight,typeshift)==1)
		{
			int length=(ReadUniversalCode2(&dec,size-pos-3,lengthweights1,lengthshift1,lengthweights2,lengthshift2)+3);
			int offs=(ReadUniversalCode2(&dec,pos-1,offsetweights1,offsetshift1,offsetweights2,offsetshift2)+1);

			CopyMemory(&buf[pos],&buf[pos-offs],length);
			pos+=length;
		}
		else
		{
			buf[pos++]=ReadBitString(&dec,8,literalbitweights,literalshift);
		}
	}
}




int main(int argc,char **argv)
{
	uint32_t size;
	size=fgetc(stdin);
	size|=fgetc(stdin)<<8;
	size|=fgetc(stdin)<<16;
	size|=fgetc(stdin)<<24;

	uint32_t shifts;
	shifts=fgetc(stdin);
	shifts|=fgetc(stdin)<<8;
	shifts|=fgetc(stdin)<<16;

	uint8_t *buf=malloc(size);

	DecompressData(stdin,buf,size,shifts>>20,(shifts>>16)&0xf,(shifts>>12)&0xf,(shifts>>8)&0xf,(shifts>>4)&0xf,shifts&0xf);

	fwrite(buf,size,1,stdout);
}
