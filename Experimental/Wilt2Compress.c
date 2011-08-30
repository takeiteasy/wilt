#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "RangeCoder/RangeEncoder.h"




typedef struct DictionaryEntry
{
	uint32_t dataoffset,nextoffset;
} DictionaryEntry;

typedef struct DictionaryLookup
{
	uint8_t *buf;
	uint32_t size;
	DictionaryEntry *entries;
	uint32_t offsets[65536];
} DictionaryLookup;

static inline uint16_t GetUInt16LE(uint8_t *ptr)
{
	return ptr[0]+(ptr[1]<<8);
}

static void InitializeDictionaryLookup(DictionaryLookup *self,uint8_t *buf,uint32_t size)
{
	self->buf=buf;
	self->size=size;
	self->entries=malloc(size*sizeof(DictionaryEntry));
	memset(self->offsets,0xff,sizeof(self->offsets));

	for(int i=size-3;i>=0;i--)
	{
		uint16_t val=GetUInt16LE(&buf[i]);

		DictionaryEntry *entry=&self->entries[i];
		entry->dataoffset=i;
		entry->nextoffset=self->offsets[val];
		self->offsets[val]=i;
	}
}

static bool FindDictionaryMatch(DictionaryLookup *self,int start,int *length,int *offs)
{
	int maxlength=0,maxpos=-1;

	uint16_t first=GetUInt16LE(&self->buf[start]);
	uint32_t entryoffset=self->offsets[first];

	while(entryoffset!=0xffffffff && self->entries[entryoffset].dataoffset<start)
	{
		int pos=self->entries[entryoffset].dataoffset;
		int matchlen=2;
		while(pos+matchlen+2<=self->size && start+matchlen+1<=self->size
		&& self->buf[pos+matchlen]==self->buf[start+matchlen]) matchlen+=1;

		if(matchlen>=maxlength) // Use >= to capture the *last* hit for multiples.
		{
			maxlength=matchlen;
			maxpos=pos;
		}

		entryoffset=self->entries[entryoffset].nextoffset;
	}

	if(maxlength>=3)
	{
		*length=maxlength;
		*offs=maxpos;
		return true;
	}
	else return false;
}




void CompressData(FILE *fh,uint8_t *buf,uint32_t size,
int typeshift,int literalshift,int lengthshift1,int lengthshift2,int offsetshift1,int offsetshift2)
{
	RangeEncoder comp;
	InitializeRangeEncoder(&comp,STDIOWriteFunction,fh);

	DictionaryLookup dict;
	InitializeDictionaryLookup(&dict,buf,size);

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
		if(FindDictionaryMatch(&dict,pos,&length,&offs))
		{
			WriteDynamicBit(&comp,1,&typeweight,typeshift);
			WriteUniversalCode2(&comp,length-3,size-pos-3,lengthweights1,lengthshift1,lengthweights2,lengthshift2);
			WriteUniversalCode2(&comp,(pos-offs)-1,pos-1,offsetweights1,offsetshift1,offsetweights2,offsetshift2);

			pos+=length;
		}
		else
		{
			WriteDynamicBit(&comp,0,&typeweight,typeshift);
			WriteBitString(&comp,buf[pos],8,literalbitweights,literalshift);

			pos+=1;
		}
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
	if(argc!=1&&argc!=7)
	{
		fprintf(stderr,"Usage: %s <inputfile >outputfile [typeshift literalshift lengthshift1 lengthshift2 offsetshift1 offsetshift2]\n",argv[0]);
		exit(1);
	}

	uint32_t size;
	uint8_t *file=AllocAndReadFile(stdin,&size);

	int typeshift=4,literalshift=2,lengthshift1=4,lengthshift2=4,offsetshift1=4,offsetshift2=4;
	if(argc==7)
	{
		typeshift=atoi(argv[1]);
		literalshift=atoi(argv[2]);
		lengthshift1=atoi(argv[3]);
		lengthshift2=atoi(argv[4]);
		offsetshift1=atoi(argv[5]);
		offsetshift2=atoi(argv[6]);
	}

	fputc(size&0xff,stdout);
	fputc((size>>8)&0xff,stdout);
	fputc((size>>16)&0xff,stdout);
	fputc((size>>24)&0xff,stdout);

	fputc((offsetshift1<<4)|offsetshift2,stdout);
	fputc((lengthshift1<<4)|lengthshift2,stdout);
	fputc((typeshift<<4)|literalshift,stdout);

	CompressData(stdout,file,size,typeshift,literalshift,lengthshift1,lengthshift2,offsetshift1,offsetshift2);
}
