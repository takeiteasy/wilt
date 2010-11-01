#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "RangeCoder/RangeEncoder.h"



void CompressData(FILE *fh,uint8_t *buf,uint32_t size,int shift)
{
	RangeEncoder comp;
	InitRangeEncoder(&comp,fh);

	int weights[256][256];
	for(int i=0;i<256;i++)
	for(int j=0;j<256;j++)
	weights[i][j]=0x800;

	int lastbyte=0;
	for(int pos=0;pos<size;pos++)
	{
		WriteBitString(&comp,buf[pos],8,weights[lastbyte],shift);
		lastbyte=buf[pos];
	}

	FinishRangeEncoder(&comp);
}




uint8_t *AllocAndReadFile(FILE *fh,uint32_t *size)
{
	const int blocksize=4096;
	uint8_t *buf=malloc(blocksize);
	uint32_t total=0;
	for(;;)
	{
		uint32_t actual=fread(buf+total,1,blocksize,fh);
		total+=actual;
		if(actual<blocksize) break;
		buf=realloc(buf,total+blocksize);
	}

	*size=total;
	return buf;
}

int main(int argc,char **argv)
{
	if(argc!=1&&argc!=2)
	{
		fprintf(stderr,"Usage: %s <inputfile >outputfile [shift]\n",argv[0]);
		exit(1);
	}

	uint32_t size;
	uint8_t *file=AllocAndReadFile(stdin,&size);

	int shift=4;
	if(argc==2)
	{
		shift=atoi(argv[1]);
	}

	fputc(size&0xff,stdout);
	fputc((size>>8)&0xff,stdout);
	fputc((size>>16)&0xff,stdout);
	fputc((size>>24)&0xff,stdout);

	fputc(shift,stdout);

	CompressData(stdout,file,size,shift);
}
