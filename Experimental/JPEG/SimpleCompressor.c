#include "Compressor.h"

#include <stdio.h>

int main(int argc,const char **argv)
{
	if(argc!=3)
	{
		fprintf(stderr,"Usage: %s infile.jpg outfile.data\n",argv[0]);
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

	fseek(in,0,SEEK_END);
	long size=ftell(in);
	fseek(in,0,SEEK_SET);

	void *bytes=malloc(size);
	if(!bytes)
	{
		fprintf(stderr,"Out of memory.\n");
		exit(1);
	}

	if(fread(bytes,1,size,in)!=size)
	{
		fprintf(stderr,"Error reading from file \"%s\".\n",argv[1]);
		exit(1);
	}

	fclose(in);

	JPEGCompressor *compressor=AllocJPEGCompressor(bytes,size);
	if(!compressor)
	{
		fprintf(stderr,"Out of memory.\n");
		exit(1);
	}

	TestJPEGCompressor(compressor,out);

	FreeJPEGCompressor(compressor);

	return 0;
}

