#include "Decompressor.h"

#include <stdio.h>

int main(int argc,const char **argv)
{
	if(argc!=1)
	{
		fprintf(stderr,"Usage: %s <input.data >output.jpg\n",argv[0]);
		exit(1);
	}

	JPEGDecompressor *decompressor=AllocJPEGDecompressor(STDIOReadFunction,stdin);
	if(!decompressor)
	{
		fprintf(stderr,"Out of memory.\n");
		exit(1);
	}

	int error=ReadJPEGHeader(decompressor);
	if(error)
	{
		fprintf(stderr,"Error %d while trying to read header.\n",error);
		exit(1);
	}

	for(;;)
	{
		error=ReadNextJPEGBundle(decompressor);
		if(error)
		{
			fprintf(stderr,"Error %d while trying to read next bundle.\n",error);
			exit(1);
		}

		fwrite(JPEGBundleMetadataBytes(decompressor),
		1,JPEGBundleMetadataLength(decompressor),stdout);

		if(IsFinalJPEGBundle(decompressor)) break;

		uint8_t buffer[1024];
		for(;;)
		{
			size_t actual=EncodeJPEGBlocksToBuffer(decompressor,buffer,sizeof(buffer));
			if(!actual) break;
			fwrite(buffer,1,actual,stdout);
		}
	}

	FreeJPEGDecompressor(decompressor);

	return 0;
}

