#include "Decompressor.h"

#include <stdio.h>

int main(int argc,const char **argv)
{
	if(argc!=3)
	{
		fprintf(stderr,"Usage: %s infile.dat outfile.jpg\n",argv[0]);
		exit(1);
	}

	FILE *in=fopen(argv[1],"rb");
	if(!in)
	{
		fprintf(stderr,"Couldn't open file \"%s\" for reading.\n",argv[1]);
		exit(1);
	}

	FILE *out=fopen(argv[2],"wb");
	if(!out)
	{
		fprintf(stderr,"Couldn't open file \"%s\" for writing.\n",argv[2]);
		exit(1);
	}

	JPEGDecompressor *decompressor=AllocJPEGDecompressor();
	if(!decompressor)
	{
		fprintf(stderr,"Out of memory.\n");
		exit(1);
	}

	TestJPEGDecompressor(decompressor,in,out);

	FreeJPEGDecompressor(decompressor);

	return 0;
}

