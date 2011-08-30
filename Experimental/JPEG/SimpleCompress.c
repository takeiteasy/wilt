#include "Compressor.h"

#include <stdio.h>

static void *AllocAndReadFile(FILE *fh,size_t *size);

int main(int argc,const char **argv)
{
	if(argc!=1)
	{
		fprintf(stderr,"Usage: %s <input.jpg >output.data\n",argv[0]);
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

	RunJPEGCompressor(compressor,stdout);

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
