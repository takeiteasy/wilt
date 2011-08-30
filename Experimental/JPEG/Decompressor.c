#include "Decompressor.h"
#include "HuffmanWriter.h"
#include "Primitives.h"

#include <stdlib.h>
#include <string.h>

//
// Constructor and destructor.
//

JPEGDecompressor *AllocJPEGDecompressor(RangeDecoderReadFunction *readfunc,void *readcontext)
{
	int b1=readfunc(readcontext); if(b1<0) return NULL;
	int b2=readfunc(readcontext); if(b2<0) return NULL;
	int b3=readfunc(readcontext); if(b3<0) return NULL;
	int b4=readfunc(readcontext); if(b4<0) return NULL;

	JPEGDecompressor *self=calloc(sizeof(JPEGDecompressor),1);
	if(!self) return NULL;

	InitializeRangeDecoder(&self->decoder,readfunc,readcontext);

	self->metadatalength=0;
	self->metadatabytes=NULL;

	self->isfirstbundle=true;
	self->reachedend=false;

	uint32_t shifts=(b1<<24)|(b2<<16)|(b3<<8)|b4;

	self->eobshift=(shifts&7)+1;
	self->zeroshift=((shifts>>3)&7)+1;
	self->pivotshift=((shifts>>6)&7)+1;
	self->acmagnitudeshift=((shifts>>9)&7)+1;
	self->acremaindershift=((shifts>>12)&7)+1;
	self->acsignshift=((shifts>>15)&7)+1;
	self->dcmagnitudeshift=((shifts>>18)&7)+1;
	self->dcremaindershift=((shifts>>21)&7)+1;
	self->dcsignshift=((shifts>>24)&7)+1;

	memset(self->rows,0,sizeof(self->rows));

	self->mcusavailable=false;

	return self;
}

void FreeJPEGDecompressor(JPEGDecompressor *self)
{
	if(!self) return;

	free(self->metadatabytes);

	for(int i=0;i<4;i++)
	for(int j=0;j<3;j++)
	free(self->rows[i][j]);

	free(self);
}




int ReadJPEGHeader(JPEGDecompressor *self)
{
/*	// Read 4-byte header.
	uint8_t header[4];
	int error=FullRead(self,header,sizeof(header));
	if(error) return error;

	// Sanity check the header, and make sure it contains only versions we can handle.
	if(header[0]<4) return JPEGInvalidHeaderError;
	if(header[1]!=0x10) return JPEGInvalidHeaderError;
	if(header[2]!=0x01) return JPEGInvalidHeaderError;
	if(header[3]&0xe0) return JPEGInvalidHeaderError;

	// The header can possibly be bigger than 4 bytes, so skip the rest.
	// (Unlikely to happen).
	if(header[0]>4)
	{
		int error=SkipBytes(self,header[0]-4);
		if(error) return error;
	}

	// Parse slice value.
	self->slicevalue=header[3]&0x1f;
*/
	return JPEGNoError;
}



// Bundle reading.

static void InitializeContexts(int *contexts,size_t totalbytes);

int ReadNextJPEGBundle(JPEGDecompressor *self)
{
	// Free and clear any old metadata.
	free(self->metadatabytes);
	self->metadatalength=0;
	self->metadatabytes=NULL;

	// Free and clear old rows.
	for(int i=0;i<4;i++)
	for(int j=0;j<3;j++)
	free(self->rows[i][j]);
	memset(self->rows,0,sizeof(self->rows));

	// Read bundle header.
	uint32_t size=0;
	for(int i=0;i<24;i++) size=(size<<1)|ReadBit(&self->decoder,0x800);

	// Allocate space for the uncompressed metadata.
	self->metadatabytes=malloc(size);
	if(!self->metadatabytes) return JPEGOutOfMemoryError;
	self->metadatalength=size;

	// Uncompress metadata.
	for(int i=0;i<self->metadatalength;i++)
	{
		int byte=0;
		for(int j=0;j<8;j++) byte=(byte<<1)|ReadBit(&self->decoder,0x800);
		self->metadatabytes[i]=byte;
	}

	// Parse the JPEG structure. If this is the first bundle,
	// we have to first find the start marker.
	const uint8_t *metadatastart;
	if(self->isfirstbundle)
	{
		metadatastart=FindStartOfJPEGImage(self->metadatabytes,self->metadatalength);
		if(!metadatastart) return JPEGParseError;

		self->isfirstbundle=false;
	}
	else
	{
		metadatastart=self->metadatabytes;
	}

	int parseres=ParseJPEGMetadata(&self->jpeg,metadatastart,
	self->metadatabytes+self->metadatalength-metadatastart);
	if(parseres==JPEGMetadataParsingFailed) return JPEGParseError;

	// If we encountered an End Of Image marker, there will be
	// no further scans or bundles, so set a flag and return.
	if(parseres==JPEGMetadataFoundEndOfImage)
	{
		self->reachedend=true;
		return JPEGNoError;
	}

	// Initialize arithmetic decoder contexts.
	InitializeContexts(&self->eobbins[0][0][0],sizeof(self->eobbins));
	InitializeContexts(&self->zerobins[0][0][0][0],sizeof(self->zerobins));
	InitializeContexts(&self->pivotbins[0][0][0][0],sizeof(self->pivotbins));
	InitializeContexts(&self->acmagnitudebins[0][0][0][0][0],sizeof(self->acmagnitudebins));
	InitializeContexts(&self->acremainderbins[0][0][0][0],sizeof(self->acremainderbins));
	InitializeContexts(&self->acsignbins[0][0][0][0],sizeof(self->acsignbins));
	InitializeContexts(&self->dcmagnitudebins[0][0][0],sizeof(self->dcmagnitudebins));
	InitializeContexts(&self->dcremainderbins[0][0][0],sizeof(self->dcremainderbins));
	InitializeContexts(&self->dcsignbins[0][0][0][0],sizeof(self->dcsignbins));

	// Allocate memory for a few rows of data, for neighbour blocks.
	for(int i=0;i<self->jpeg.numscancomponents;i++)
	for(int j=0;j<3;j++)
	{
		self->rows[i][j]=malloc(self->jpeg.horizontalmcus*
		self->jpeg.scancomponents[i].component->horizontalfactor*
		sizeof(JPEGBlock));

		if(!self->rows[i][j]) return JPEGOutOfMemoryError;
	}

	self->mcusavailable=true;
	self->neednewmcu=true;

	self->mcurow=0;
	self->mcucol=0;
	self->mcucomp=0;
	self->mcux=0;
	self->mcuy=0;
	self->mcucoeff=0;

	self->mcucounter=0;
	self->restartmarkerindex=0;

	memset(self->predicted,0,sizeof(self->predicted));

	InitializeBitStreamWriter(&self->bitstream);

	return JPEGNoError;
}

static void InitializeContexts(int *contexts,size_t totalbytes)
{
	for(int i=0;i<totalbytes/sizeof(*contexts);i++) contexts[i]=0x800;
}



//
// Block decompressing and recompressing.
//

// Decompressor functions.
static void DecompressBlock(JPEGDecompressor *self,int comp,
JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static int DecompressACComponent(JPEGDecompressor *self,int comp,unsigned int k,bool canbezero,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static int DecompressACSign(JPEGDecompressor *self,int comp,unsigned int k,int absvalue,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static int DecompressDCComponent(JPEGDecompressor *self,int comp,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static const JPEGBlock ZeroBlock={0};



size_t EncodeJPEGBlocksToBuffer(JPEGDecompressor *self,void *bytes,size_t length)
{
	uint8_t	*start=bytes;
	uint8_t	*ptr=bytes;
	uint8_t *end=ptr+length;

	while(ptr<end)
	{
		if(ByteAvailableFromBitStream(&self->bitstream))
		{
			*ptr++=OutputByteFromBitStream(&self->bitstream);
		}
		else if(self->mcusavailable)
		{
			if(self->neednewmcu)
			{
				// Decompress the new block if needed.
				self->neednewmcu=false;

				int hblocks=self->jpeg.scancomponents[self->mcucomp].component->horizontalfactor;
				int vblocks=self->jpeg.scancomponents[self->mcucomp].component->verticalfactor;
				JPEGQuantizationTable *quantization=self->jpeg.scancomponents[self->mcucomp].component->quantizationtable;

				int x=self->mcucol*hblocks+self->mcux;
				int y=self->mcurow*vblocks+self->mcuy;

				self->currblock=&self->rows[self->mcucomp][y%3][x];

				JPEGBlock *west;
				if(x==0) west=NULL;
				else west=&self->rows[self->mcucomp][y%3][x-1];

				JPEGBlock *north;
				if(y==0) north=NULL;
				else north=&self->rows[self->mcucomp][(y-1)%3][x];

				DecompressBlock(self,self->mcucomp,self->currblock,north,west,quantization);

				// TODO: Better EOF handling.
				if(RangeDecoderReachedEOF(&self->decoder))
				{
					self->mcusavailable=false;
					continue;
				}
			}

			if(self->jpeg.restartinterval && self->mcucounter==self->jpeg.restartinterval)
			{
				// If it is time for a restart marker, output one.
				FlushAndPushRestartMarker(&self->bitstream,
				self->restartmarkerindex);

				// Cycle the restart marker indexes, reset the MCU counter, and clear predictors.
				self->restartmarkerindex=(self->restartmarkerindex+1)&7;
				self->mcucounter=0;
				memset(self->predicted,0,sizeof(self->predicted));
			}
			else if(self->mcucoeff==0)
			{
				// Output DC coefficient.
				int diff=self->currblock->c[0]-self->predicted[self->mcucomp];

				PushEncodedValue(&self->bitstream,
				self->jpeg.scancomponents[self->mcucomp].dctable,diff,0);

				self->predicted[self->mcucomp]=self->currblock->c[0];
				self->mcucoeff=1;
			}
			else if(self->mcucoeff>self->currblock->eob && self->currblock->eob!=63)
			{
				// Output EOB marker.
				PushHuffmanCode(&self->bitstream,
				self->jpeg.scancomponents[self->mcucomp].actable,0x00);

				self->mcucoeff=64;
			}
			else
			{
				// Output AC coefficient.

				// Find the next non-zero coefficient.
				int firstcoeff=self->mcucoeff;
				int endrun=self->mcucoeff+15;
				while(self->mcucoeff<63 && self->mcucoeff<endrun &&
				self->currblock->c[self->mcucoeff]==0) self->mcucoeff++;

				int zeroes=self->mcucoeff-firstcoeff;
				int val=self->currblock->c[self->mcucoeff];

				PushEncodedValue(&self->bitstream,
				self->jpeg.scancomponents[self->mcucomp].actable,val,zeroes);

				self->mcucoeff++;
			}

			// If we have output all coefficients, update position.
			if(self->mcucoeff>=64)
			{
				int hblocks=self->jpeg.scancomponents[self->mcucomp].component->horizontalfactor;
				int vblocks=self->jpeg.scancomponents[self->mcucomp].component->verticalfactor;

				self->mcucoeff=0; self->mcux++;
				if(self->mcux>=hblocks)
				{
					self->mcux=0; self->mcuy++;
					if(self->mcuy>=vblocks)
					{
						self->mcuy=0; self->mcucomp++;
						if(self->mcucomp>=self->jpeg.numscancomponents)
						{
							self->mcucomp=0; self->mcucol++;
							if(self->mcucol>=self->jpeg.horizontalmcus)
							{
								self->mcucol=0; self->mcurow++;
								if(self->mcurow>=self->jpeg.verticalmcus)
								{
									self->mcusavailable=false;
									FlushBitStream(&self->bitstream);
								}
							}

							// Count up towards the restart interval.
							self->mcucounter++;
						}
					}
				}

				self->neednewmcu=true;
			}
		}
		else
		{
			// Nothing left to do. Return the partial length of output data.
			return ptr-start;
		}
	}

	return length;
}



static void DecompressBlock(JPEGDecompressor *self,int comp,
JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	// Calculate EOB context.
	int average;
	if(!north&&!west) average=0;
	else if(!north) average=Sum(0,west);
	else if(!west) average=Sum(0,north);
	else average=(Sum(0,north)+Sum(0,west)+1)/2;

	int eobcontext=Min(Category(average),12);

	// Read EOB bits using binary tree.
	unsigned int eob=ReadBitString(&self->decoder,6,
	self->eobbins[comp][eobcontext],
	self->eobshift);

	current->eob=eob;

	// Fill out the elided block entries with 0.
	for(unsigned int k=eob+1;k<=63;k++) current->c[k]=0;

	// Decode AC components in decreasing order, if any. (5.6.6)
	for(unsigned int k=eob;k>=1;k--)
	{
		current->c[k]=DecompressACComponent(self,comp,k,k!=eob,current,north,west,quantization);
	}

	// Decode DC component.
	current->c[0]=DecompressDCComponent(self,comp,current,north,west,quantization);
}

static int DecompressACComponent(JPEGDecompressor *self,int comp,unsigned int k,bool canbezero,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	if(!north) north=&ZeroBlock;
	if(!west) west=&ZeroBlock;

	int val1;
	if(IsFirstRowOrColumn(k)) val1=Abs(BDR(k,current,north,west,quantization));
	else val1=Average(k,north,west,quantization);

	int val2=Sum(k,current);

	if(canbezero)
	{
		// Decode zero/non-zero bit.
		int zerocontext1=Min(Category(val1),2);
		int zerocontext2=Min(Category(val2),5);

		int nonzero=ReadDynamicBit(&self->decoder,
		&self->zerobins[comp][k-1][zerocontext1][zerocontext2],
		self->zeroshift);

		// If this component is zero, there is no need to decode further parameters.
		if(!nonzero) return 0;
	}

	// This component is not zero. Proceed with decoding absolute value.
	int absvalue;

	// Decode pivot (abs>=2).
	int pivotcontext1=Min(Category(val1),4);
	int pivotcontext2=Min(Category(val2),6);

	int pivot=ReadDynamicBit(&self->decoder,
	&self->pivotbins[comp][k-1][pivotcontext1][pivotcontext2],
	self->pivotshift);

	if(!pivot)
	{
		// The absolute of this component is not >=2. It must therefore be 1,
		// and there is no need to decode the value.
		absvalue=1;
	}
	else
	{
		// The absolute of this component is >=2. Proceed with decoding
		// the absolute value.
		int val3,n;
		if(IsFirstRow(k)) { val3=Column(k)-1; n=0; }
		else if(IsFirstColumn(k)) { val3=Row(k)-1; n=1; }
		else { val3=Category(k-4); n=2; }

		int magnitudecontext1=Min(Category(val1),8);
		int magnitudecontext2=Min(Category(val2),8);
		int remaindercontext=val3;

		// Decode absolute value.
		absvalue=ReadUniversalCode(&self->decoder,
		self->acmagnitudebins[comp][n][magnitudecontext1][magnitudecontext2],
		self->acmagnitudeshift,
		self->acremainderbins[comp][n][remaindercontext],
		self->acremaindershift)+2;
	}

	if(DecompressACSign(self,comp,k,absvalue,current,north,west,quantization)) return -absvalue;
	else return absvalue;
}

static int DecompressACSign(JPEGDecompressor *self,int comp,unsigned int k,int absvalue,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	// Calculate sign context, or decode with fixed probability.
	int predictedsign;
	if(IsFirstRowOrColumn(k))
	{
		int bdr=BDR(k,current,north,west,quantization);

		if(bdr==0) return ReadBit(&self->decoder,0x800);

		predictedsign=(bdr<0);
	}
	else if(k==4)
	{
		int sign1=Sign(north->c[k]);
		int sign2=Sign(west->c[k]);

		if(sign1+sign2==0) return ReadBit(&self->decoder,0x800);

		predictedsign=(sign1+sign2<0);
	}
	else if(IsSecondRow(k))
	{
		if(north->c[k]==0) return ReadBit(&self->decoder,0x800);

		predictedsign=(north->c[k]<0);
	}
	else if(IsSecondColumn(k))
	{
		if(west->c[k]==0) return ReadBit(&self->decoder,0x800);

		predictedsign=(west->c[k]<0);
	}
	else
	{
		return ReadBit(&self->decoder,0x800);
	}

	static const int n_for_k[64]={
		 0,
		 0, 1,
		 2, 3, 4,
		 5, 6, 7, 8,
		 9,10, 0,11,12,
		13,14, 0, 0,15,16,
		17,18, 0, 0, 0,19,20,
		21,22, 0, 0, 0, 0,23,24,
		25, 0, 0, 0, 0, 0,26,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0,
		 0, 0,
		 0,
	};
	int n=n_for_k[k];

	int signcontext1=Min(Category(absvalue)/2,2);

	return ReadDynamicBit(&self->decoder,
	&self->acsignbins[comp][n][signcontext1][predictedsign],
	self->acsignshift);
}

static int DecompressDCComponent(JPEGDecompressor *self,int comp,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	// DC prediction.
	int predicted;
	if(!north&&!west)
	{
		predicted=0;
	}
	else if(!north)
	{
		// NOTE: Spec says west[1]-current[1].
		int t1=west->c[0]*10000-11038*quantization->c[1]*(west->c[1]+current->c[1])/quantization->c[0];
		int p1=((t1<0)?(t1-5000):(t1+5000))/10000;
		predicted=p1;
	}
	else if(!west)
	{
		// NOTE: Spec says north->c[2]-current->c[2].
		int t0=north->c[0]*10000-11038*quantization->c[2]*(north->c[2]+current->c[2])/quantization->c[0];
		int p0=((t0<0)?(t0-5000):(t0+5000))/10000;
		predicted=p0;
	}
	else
	{
		// NOTE: Spec says north[2]-current[2] and west[1]-current[1].
		int t0=north->c[0]*10000-11038*quantization->c[2]*(north->c[2]+current->c[2])/quantization->c[0];
		int p0=((t0<0)?(t0-5000):(t0+5000))/10000;

		int t1=west->c[0]*10000-11038*quantization->c[1]*(west->c[1]+current->c[1])/quantization->c[0];
		int p1=((t1<0)?(t1-5000):(t1+5000))/10000;

		// Prediction refinement.
		int d0=0,d1=0;
		for(int i=1;i<8;i++)
		{
			// Note: Spec says Abs(Abs(north->c[ZigZag(i,0)])-
			// Abs(current->c[ZigZag(i,0)])) and similarly for west.
			d0+=Abs(north->c[ZigZag(i,0)]-current->c[ZigZag(i,0)]);
			d1+=Abs(west->c[ZigZag(0,i)]-current->c[ZigZag(0,i)]);
		}

		if(d0>d1)
		{
			int64_t weight=1LL<<Min(d0-d1,31);
			predicted=(weight*(int64_t)p1+(int64_t)p0)/(1+weight);
		}
		else
		{
			int64_t weight=1LL<<Min(d1-d0,31);
			predicted=(weight*(int64_t)p0+(int64_t)p1)/(1+weight);
		}
	}

	// Decode DC residual.

	// Decode absolute value.
	int absvalue;
	int sum=Sum(0,current);
	int valuecontext=Min(Category(sum),12);

	absvalue=ReadUniversalCode(&self->decoder,
	self->dcmagnitudebins[comp][valuecontext],
	self->dcmagnitudeshift,
	self->dcremainderbins[comp][valuecontext],
	self->dcremaindershift);
	if(absvalue==0) return predicted;

	// Decode sign.
	// NOTE: Spec says north[0]<0 and west[0]<0.
	if(!north) north=&ZeroBlock;
	if(!west) west=&ZeroBlock;
	int northsign=(north->c[0]<predicted);
	int westsign=(west->c[0]<predicted);
	int predictedsign=(predicted<0);

	int sign=ReadDynamicBit(&self->decoder,
	&self->dcsignbins[comp][northsign][westsign][predictedsign],
	self->dcsignshift);

	if(sign) return predicted-absvalue;
	else return predicted+absvalue;
}

