#include "LZAWDecompressDS.h"

#include <unistd.h>
#include <stdlib.h>



typedef struct RangeDecoder
{
	uint32_t range,code;
	int fd;
} RangeDecoder;

static void InitRangeDecoder(RangeDecoder *self,int fd)
{
	self->fd=fd;
	self->range=0xffffffff;
	self->code=0;

	static uint8_t buf[4];
	read(fd,buf,4);
	for(int i=0;i<4;i++) self->code=(self->code<<8)|buf[i];
}

static void NormalizeRangeDecoder(RangeDecoder *self)
{
	if(self->range<0x1000000)
	{
		uint8_t val;
		read(self->fd,&val,1);
		self->code=(self->code<<8) | val;
		self->range<<=8;
	}
}

static int ReadWeightedBitFromRangeDecoder(RangeDecoder *self,int weight,int shift)
{
	NormalizeRangeDecoder(self);

	uint32_t threshold=(self->range>>shift)*weight;

	int bit;
	if(self->code<threshold)
	{
		bit=0;
		self->range=threshold;
	}
	else
	{
		bit=1;
		self->range-=threshold;
		self->code-=threshold;
	}

	return bit;
}

static int ReadBitAndUpdateWeight(RangeDecoder *self,uint16_t *weight,int shift)
{
	int bit=ReadWeightedBitFromRangeDecoder(self,*weight,12);
	if(bit==0) *weight+=(0x1000-*weight)>>shift;
	else *weight-=*weight>>shift;
	return bit;
}

static uint32_t ReadUniversalCode(RangeDecoder *self,uint16_t *weights,int shift)
{
	int numbits=0;

	while(ReadBitAndUpdateWeight(self,&weights[numbits],shift)==1) numbits++;
	if(!numbits) return 0;

	uint32_t val=1;

	for(int i=0;i<numbits-1;i++)
	val=(val<<1)|ReadWeightedBitFromRangeDecoder(self,0x800,12);

	return val;
}




static void CopyMemory(uint16_t *dest,uint16_t *src,int length)
{
	for(int i=0;i<length;i++) *dest++=*src++;
}

void DecompressData(int fd,uint16_t *buf,uint32_t size,
int typeshift,int lengthshift,int offsshift,int litshift)
{
	static RangeDecoder dec;
	InitRangeDecoder(&dec,fd);

	uint16_t typeweight=0x800;

	uint16_t lengthweights[33];
	uint16_t offsweights[33];
	for(int i=0;i<33;i++)
	lengthweights[i]=offsweights[i]=0x800;

	uint16_t literalbitweights[16][16];
	for(int i=0;i<16;i++)
	for(int j=0;j<16;j++)
	literalbitweights[i][j]=0x800;

	int pos=0;
	while(pos<size/2)
	{
		int length,offs;

		if(ReadBitAndUpdateWeight(&dec,&typeweight,typeshift)==1)
		{
			int length=ReadUniversalCode(&dec,lengthweights,lengthshift)+2;
			int offs=ReadUniversalCode(&dec,offsweights,offsshift)+1;

			CopyMemory(&buf[pos],&buf[pos-offs],length);

			pos+=length;
		}
		else
		{
			uint16_t val=0;

			for(int i=0;i<16;i++)
			{
				int bit=ReadBitAndUpdateWeight(&dec,&literalbitweights[i][val&15],litshift);
				val=(val<<1)|bit;
			}
			buf[pos]=val;

			pos++;
		}
	}
}

#ifdef TEST
int main(int argc,char **argv)
{
	uint32_t size;
	read(0,&size,4);

	uint16_t shifts;
	read(0,&shifts,2);

	uint16_t *buf=malloc(size);
	DecompressData(0,buf,size,shifts>>12,(shifts>>8)&0xf,(shifts>>4)&0xf,shifts&0xf);
	write(1,buf,size);
}
#endif
