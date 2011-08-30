#include "Compressor.h"

#include <stdio.h>
#include <stdlib.h>

static void *AllocAndReadFile(FILE *fh,size_t *size);

int main(int argc,const char **argv)
{
	if(argc!=1&&argc!=10)
	{
		fprintf(stderr,"Usage: %s <input.jpg >output.data [eobshift zeroshift pivotshift acmagnitudeshift acremaindershift acsignshift dcmagnitudeshift dcremaindershift dcsignshift\n",argv[0]);
		exit(1);
	}

	size_t size;
	void *bytes=AllocAndReadFile(stdin,&size);
	if(!bytes)
	{
		fprintf(stderr,"Out of memory.\n");
		exit(1);
	}

	JPEGCompressor *compressor=AllocJPEGCompressor(bytes,size);
	if(!compressor)
	{
		fprintf(stderr,"Out of memory.\n");
		exit(1);
	}

	if(argc==10)
	{
		SetJPEGCompressorShifts(compressor,
		atoi(argv[1]),atoi(argv[2]),atoi(argv[3]),
		atoi(argv[4]),atoi(argv[5]),atoi(argv[6]),
		atoi(argv[7]),atoi(argv[8]),atoi(argv[9]));
	}

	if(!RunJPEGCompressor(compressor,stdout))
	{
		fprintf(stderr,"This file is not supported. Output is invalid.\n");
		exit(1);
	}

	FreeJPEGCompressor(compressor);

	return 0;
}

void *AllocAndReadFile(FILE *fh,size_t *size)
{
	const int blocksize=4096;
	char *buf=malloc(blocksize);
	size_t total=0;

	for(;;)
	{
		size_t actual=fread(buf+total,1,blocksize,fh);
		total+=actual;
		if(actual<blocksize) break;
		buf=realloc(buf,total+blocksize);
	}

	*size=total;
	return buf;
}
