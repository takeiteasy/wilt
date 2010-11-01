#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "RangeCoder/RangeDecoder.h"




static void CopyMemory(uint8_t *dest,uint8_t *src,int length)
{
	uint16_t *dest16=(uint16_t *)dest;
	uint16_t *src16=(uint16_t *)src;
	for(int i=0;i<length/2;i++) *dest16++=*src16++;
}

void DecompressData(FILE *fh,uint8_t *buf,uint32_t size,
int typeshift,int literalshift,int lengthshift1,int lengthshift2,int offsetshift1,int offsetshift2)
{
	RangeDecoder dec;
	InitRangeDecoder(&dec,fh);

	int typeweight=0x800;

	int lengthweights1[32],lengthweights2[32];
	int offsetweights1[32],offsetweights2[32];
	for(int i=0;i<32;i++)
	lengthweights1[i]=lengthweights2[i]=offsetweights1[i]=offsetweights2[i]=0x800;

	int literalbitweights[16][16];
	for(int i=0;i<16;i++)
	for(int j=0;j<16;j++)
	literalbitweights[i][j]=0x800;

	int pos=0;
	while(pos<size)
	{
		int length,offs;

		if(ReadDynamicBit(&dec,&typeweight,typeshift)==1)
		{
			int length=(ReadUniversalCode(&dec,lengthweights1,lengthshift1,lengthweights2,lengthshift2)+2)*2;
			int offs=(ReadUniversalCode(&dec,offsetweights1,offsetshift1,offsetweights2,offsetshift2)+1)*2;

			CopyMemory(&buf[pos],&buf[pos-offs],length);

			pos+=length;
		}
		else
		{
			uint16_t val=0;

			for(int i=0;i<16;i++)
			{
				int bit=ReadDynamicBit(&dec,&literalbitweights[i][val&15],literalshift);
				val=(val<<1)|bit;
			}
			buf[pos]=val&0xff;
			buf[pos+1]=val>>8;

			pos+=2;
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
