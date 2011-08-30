#include "Compressor.h"
#include "Primitives.h"

#include <stdlib.h>
#include <string.h>



//
// Constructor and destructor.
//

JPEGCompressor *AllocJPEGCompressor(const void *bytes,size_t length)
{
	JPEGCompressor *self=calloc(sizeof(JPEGCompressor),1);
	if(!self) return NULL;

	self->start=bytes;
	self->end=self->start+length;

	self->eobshift=4;
	self->zeroshift=4;
	self->pivotshift=4;
	self->acmagnitudeshift=4;
	self->acremaindershift=4;
	self->acsignshift=4;
	self->dcmagnitudeshift=4;
	self->dcremaindershift=4;
	self->dcsignshift=4;

	return self;
}

void FreeJPEGCompressor(JPEGCompressor *self)
{
	if(!self) return;

	//free(self->metadatabytes);
	//for(int i=0;i<4;i++) free(self->blocks[i]);
	free(self);
}



// Block reading.

static bool ReadBlock(JPEGCompressor *self,JPEGBlock *block,int comp);
static int DecodeValue(int magnitude,int remainder);

// Block compression.

static void InitializeContexts(int *contexts,size_t totalbytes);

static void CompressBlock(JPEGCompressor *self,int comp,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static void CompressACComponent(JPEGCompressor *self,int comp,unsigned int k,bool canbezero,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static void CompressACSign(JPEGCompressor *self,int comp,unsigned int k,int absvalue,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static void CompressDCComponent(JPEGCompressor *self,int comp,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static const JPEGBlock ZeroBlock={0};




bool RunJPEGCompressor(JPEGCompressor *self,FILE *output)
{
	// Parse the JPEG structure. If this is the first bundle,
	// we have to first find the start marker.

	int parseres=ParseJPEGMetadata(&self->jpeg,self->start,self->end-self->start);
	if(parseres==JPEGMetadataParsingFailed) return false;
	if(parseres==JPEGMetadataFoundEndOfImage) return false;

	// TODO: Do sanity and conformance checks here.

	InitializeBitStreamReader(&self->bitstream,self->start+self->jpeg.bytesparsed,
	self->end-self->start-self->jpeg.bytesparsed);

	// Initialize Huffman lookup tables.
	for(int i=0;i<4;i++)
	{
		if(self->jpeg.huffmandefined[0][i])
		{
			InitializeHuffmanTable(&self->dctables[i]);
			for(int j=0;j<256;j++)
			{
				if(self->jpeg.huffmantables[0][i].codes[j].length)
				AddHuffmanCode(&self->dctables[i],
				self->jpeg.huffmantables[0][i].codes[j].code,
				self->jpeg.huffmantables[0][i].codes[j].length,j);
			}
		}

		if(self->jpeg.huffmandefined[1][i])
		{
			InitializeHuffmanTable(&self->actables[i]);
			for(int j=0;j<256;j++)
			{
				if(self->jpeg.huffmantables[1][i].codes[j].length)
				AddHuffmanCode(&self->actables[i],
				self->jpeg.huffmantables[1][i].codes[j].code,
				self->jpeg.huffmantables[1][i].codes[j].length,j);
			}
		}
	}

	JPEGBlock rows[4][3][self->jpeg.horizontalmcus];

	memset(self->predicted,0,sizeof(self->predicted));

	InitializeRangeEncoder(&self->encoder,STDIOWriteFunction,output);

	// Initialize arithmetic encoder contexts.
	InitializeContexts(&self->eobbins[0][0][0],sizeof(self->eobbins));
	InitializeContexts(&self->zerobins[0][0][0][0],sizeof(self->zerobins));
	InitializeContexts(&self->pivotbins[0][0][0][0],sizeof(self->pivotbins));
	InitializeContexts(&self->acmagnitudebins[0][0][0][0][0],sizeof(self->acmagnitudebins));
	InitializeContexts(&self->acremainderbins[0][0][0][0],sizeof(self->acremainderbins));
	InitializeContexts(&self->acsignbins[0][0][0][0],sizeof(self->acsignbins));
	InitializeContexts(&self->dcmagnitudebins[0][0][0],sizeof(self->dcmagnitudebins));
	InitializeContexts(&self->dcremainderbins[0][0][0],sizeof(self->dcremainderbins));
	InitializeContexts(&self->dcsignbins[0][0][0][0],sizeof(self->dcsignbins));

	int restartcounter=0;
	int restartindex=0;
	for(int row=0;row<self->jpeg.verticalmcus;row++)
	for(int col=0;col<self->jpeg.horizontalmcus;col++)
	{
		if(self->jpeg.restartinterval&&restartcounter==self->jpeg.restartinterval)
		{
			if(!FlushAndSkipRestartMarker(&self->bitstream,restartindex)) return false;
			restartindex=(restartindex+1)&7;
			restartcounter=0;
			memset(self->predicted,0,sizeof(self->predicted));
		}

		restartcounter++;

		for(int comp=0;comp<self->jpeg.numscancomponents;comp++)
		{
			int hblocks=self->jpeg.scancomponents[comp].component->horizontalfactor;
			int vblocks=self->jpeg.scancomponents[comp].component->verticalfactor;
			JPEGQuantizationTable *quantization=self->jpeg.scancomponents[comp].component->quantizationtable;

			for(int mcu_y=0;mcu_y<vblocks;mcu_y++)
			for(int mcu_x=0;mcu_x<hblocks;mcu_x++)
			{
				int x=col*hblocks+mcu_x;
				int y=row*vblocks+mcu_y;

				JPEGBlock *current=&rows[comp][y%3][x];

				JPEGBlock *west;
				if(x==0) west=NULL;
				else west=&rows[comp][y%3][x-1];

				JPEGBlock *north;
				if(y==0) north=NULL;
				else north=&rows[comp][(y-1)%3][x];

				if(!ReadBlock(self,current,comp)) return false;

//fprintf(stderr,"%d,%d,%d: ",x,y,comp);
//for(int i=0;i<64;i++) fprintf(stderr,"%d ",current->c[i]);
//fprintf(stderr,"\n");

				CompressBlock(self,comp,current,north,west,quantization);
			}
		}
	}

	FinishRangeEncoder(&self->encoder);

	return true;
}



//
// Block reading
//

static bool ReadBlock(JPEGCompressor *self,JPEGBlock *block,int comp)
{
	int dcindex=self->jpeg.scancomponents[comp].dcindex;
	int acindex=self->jpeg.scancomponents[comp].acindex;

	int dcmagnitude=ReadHuffmanCode(&self->bitstream,&self->dctables[dcindex]);
	if(dcmagnitude<0) return false;
	int dcremainder=ReadBitString(&self->bitstream,dcmagnitude);
	if(dcremainder<0) return false;

	int delta=DecodeValue(dcmagnitude,dcremainder);
	block->c[0]=self->predicted[comp]+delta;
	self->predicted[comp]+=delta;

	int coeff=1;
	while(coeff<64)
	{
		int val=ReadHuffmanCode(&self->bitstream,&self->actables[acindex]);
		if(val<0) return false;
		if(val==0x00) break;

		int zeroes=val>>4;
		int acmagnitude=val&0x0f;
		int acremainder=ReadBitString(&self->bitstream,acmagnitude);
		if(acremainder<0) return false;

		// TODO: range checks
		for(int i=0;i<zeroes;i++) block->c[coeff++]=0;
		block->c[coeff++]=DecodeValue(acmagnitude,acremainder);
	}

	for(int i=coeff;i<64;i++) block->c[i]=0;

	block->eob=coeff-1;

	return true;
}

static int DecodeValue(int magnitude,int remainder)
{
	if(magnitude==0) return 0;
	else if(remainder&(1<<magnitude-1)) return remainder;
	else return remainder-(1<<magnitude)+1;
}




//
// Block compression.
//

static void InitializeContexts(int *contexts,size_t totalbytes)
{
	for(int i=0;i<totalbytes/sizeof(*contexts);i++) contexts[i]=0x800;
}

static void CompressBlock(JPEGCompressor *self,int comp,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	// Calculate EOB context.
	int average;
	if(!north&&!west) average=0;
	else if(!north) average=Sum(0,west);
	else if(!west) average=Sum(0,north);
	else average=(Sum(0,north)+Sum(0,west)+1)/2;

	int eobcontext=Min(Category(average),12);

	// Write EOB bits using binary tree.
	WriteBitString(&self->encoder,current->eob,6,
	self->eobbins[comp][eobcontext],
	self->eobshift);

	// Compress AC components in decreasing order, if any.
	for(unsigned int k=current->eob;k>=1;k--)
	{
		CompressACComponent(self,comp,k,k!=current->eob,current,north,west,quantization);
	}

	// Compress DC component.
	CompressDCComponent(self,comp,current,north,west,quantization);
}

static void CompressACComponent(JPEGCompressor *self,int comp,unsigned int k,bool canbezero,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	int value=current->c[k];

	if(!north) north=&ZeroBlock;
	if(!west) west=&ZeroBlock;

	int val1;
	if(IsFirstRowOrColumn(k)) val1=Abs(BDR(k,current,north,west,quantization));
	else val1=Average(k,north,west,quantization);

	int val2=Sum(k,current);

	if(canbezero)
	{
		// Compress zero/non-zero bit.
		int zerocontext1=Min(Category(val1),2);
		int zerocontext2=Min(Category(val2),5);

		WriteDynamicBit(&self->encoder,value!=0,
		&self->zerobins[comp][k-1][zerocontext1][zerocontext2],
		self->zeroshift);

		// If this component is zero, there is no need to compress further parameters.
		if(value==0) return;
	}

	// This component is not zero. Proceed with compressing absolute value.
	int absvalue=Abs(value);

	// Compress pivot (abs>=2).
	int pivotcontext1=Min(Category(val1),4);
	int pivotcontext2=Min(Category(val2),6);

	WriteDynamicBit(&self->encoder,absvalue>1,
	&self->pivotbins[comp][k-1][pivotcontext1][pivotcontext2],
	self->pivotshift);

	if(absvalue>1)
	{
		// The absolute of this component is >=2. Proceed with compressing
		// the absolute value.
		int val3,n;
		if(IsFirstRow(k)) { val3=Column(k)-1; n=0; }
		else if(IsFirstColumn(k)) { val3=Row(k)-1; n=1; }
		else { val3=Category(k-4); n=2; }

		int magnitudecontext1=Min(Category(val1),8);
		int magnitudecontext2=Min(Category(val2),8);
		int remaindercontext=val3;

		// Compress absolute value.
		WriteUniversalCode(&self->encoder,absvalue-2,
		self->acmagnitudebins[comp][n][magnitudecontext1][magnitudecontext2],
		self->acmagnitudeshift,
		self->acremainderbins[comp][n][remaindercontext],
		self->acremaindershift);
	}

	CompressACSign(self,comp,k,absvalue,current,north,west,quantization);
}

static void CompressACSign(JPEGCompressor *self,int comp,unsigned int k,int absvalue,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	int sign=current->c[k]<0;

	// Calculate sign context, or compress with fixed probability.
	int predictedsign;
	if(IsFirstRowOrColumn(k))
	{
		int bdr=BDR(k,current,north,west,quantization);

		if(bdr==0)
		{
			WriteBit(&self->encoder,sign,0x800);
			return;
		}

		predictedsign=(bdr<0);
	}
	else if(k==4)
	{
		int sign1=Sign(north->c[k]);
		int sign2=Sign(west->c[k]);

		if(sign1+sign2==0)
		{
			WriteBit(&self->encoder,sign,0x800);
			return;
		}

		predictedsign=(sign1+sign2<0);
	}
	else if(IsSecondRow(k))
	{
		if(north->c[k]==0)
		{
			WriteBit(&self->encoder,sign,0x800);
			return;
		}

		predictedsign=(north->c[k]<0);
	}
	else if(IsSecondColumn(k))
	{
		if(west->c[k]==0)
		{
			WriteBit(&self->encoder,sign,0x800);
			return;
		}

		predictedsign=(west->c[k]<0);
	}
	else
	{
		WriteBit(&self->encoder,sign,0x800);
		return;
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

	WriteDynamicBit(&self->encoder,sign,
	&self->acsignbins[comp][n][signcontext1][predictedsign],
	self->acsignshift);
}

static void CompressDCComponent(JPEGCompressor *self,int comp,
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

		// Prediction refinement. (5.6.7.2)
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

	// Compress DC residual.
	int value=current->c[0]-predicted;

	// Compress absolute value.
	int absvalue=Abs(value);
	int sum=Sum(0,current);
	int valuecontext=Min(Category(sum),12);

	WriteUniversalCode(&self->encoder,absvalue,
	self->dcmagnitudebins[comp][valuecontext],
	self->dcmagnitudeshift,
	self->dcremainderbins[comp][valuecontext],
	self->dcremaindershift);
	if(absvalue==0) return;

	// Decode sign.
	// NOTE: Spec says north[0]<0 and west[0]<0.
	if(!north) north=&ZeroBlock;
	if(!west) west=&ZeroBlock;
	int northsign=(north->c[0]<predicted);
	int westsign=(west->c[0]<predicted);
	int predictedsign=(predicted<0);

	WriteDynamicBit(&self->encoder,value<0,
	&self->dcsignbins[comp][northsign][westsign][predictedsign],
	self->dcsignshift);
}

