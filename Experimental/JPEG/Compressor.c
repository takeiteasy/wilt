#include "Compressor.h"

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

	return self;
}

void FreeJPEGCompressor(JPEGCompressor *self)
{
	if(!self) return;

	//free(self->metadatabytes);
	//for(int i=0;i<4;i++) free(self->blocks[i]);
	free(self);
}


static int DecodeValue(int magnitude,int remainder)
{
	if(magnitude==0) return 0;
	else if(remainder&(1<<magnitude-1)) return remainder;
	else return remainder-(1<<magnitude)+1;
}

bool TestJPEGCompressor(JPEGCompressor *self,FILE *output)
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

	int predicted[4]={0};

	int restartcounter=0;
	int restartindex=0;
	for(int row=0;row<self->jpeg.verticalmcus;row++)
	for(int col=0;col<self->jpeg.horizontalmcus;col++)
	{
		if(self->jpeg.restartinterval&&restartcounter==self->jpeg.restartinterval)
		{
			if(!FlushBitStreamAndSkipRestartMarker(&self->bitstream,restartindex)) return false;
			restartindex=(restartindex+1)&7;
			restartcounter=0;
			memset(predicted,0,sizeof(predicted));
		}

		restartcounter++;

		for(int comp=0;comp<self->jpeg.numscancomponents;comp++)
		{
			int hblocks=self->jpeg.scancomponents[comp].component->horizontalfactor;
			int vblocks=self->jpeg.scancomponents[comp].component->verticalfactor;
			int dcindex=self->jpeg.scancomponents[comp].dcindex;
			int acindex=self->jpeg.scancomponents[comp].acindex;

			for(int mcu_y=0;mcu_y<vblocks;mcu_y++)
			for(int mcu_x=0;mcu_x<hblocks;mcu_x++)
			{
				int x=col*hblocks+mcu_x;
				int y=row*vblocks+mcu_y;

				JPEGBlock testblock;
				JPEGBlock *currblock=&testblock;

				int dcmagnitude=ReadHuffmanCode(&self->bitstream,&self->dctables[dcindex]);
				if(dcmagnitude<0) return false;
				int dcremainder=ReadBitString(&self->bitstream,dcmagnitude);
				if(dcremainder<0) return false;

				int delta=DecodeValue(dcmagnitude,dcremainder);
				currblock->c[0]=predicted[comp]+delta;
				predicted[comp]+=delta;

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
					for(int i=0;i<zeroes;i++) currblock->c[coeff++]=0;
					currblock->c[coeff++]=DecodeValue(acmagnitude,acremainder);
				}

				for(int i=coeff;i<64;i++) currblock->c[i]=0;

				currblock->eob=coeff-1;

fprintf(stderr,"%d,%d,%d: ",x,y,comp);
for(int i=0;i<64;i++) fprintf(stderr,"%d ",currblock->c[i]);
fprintf(stderr,"\n");
			}
		}
	}

	// Initialize arithmetic decoder contexts.
/*	InitializeJPEGContexts(&self->eobbins[0][0][0],sizeof(self->eobbins));
	InitializeJPEGContexts(&self->zerobins[0][0][0][0],sizeof(self->zerobins));
	InitializeJPEGContexts(&self->pivotbins[0][0][0][0],sizeof(self->pivotbins));
	InitializeJPEGContexts(&self->acmagnitudebins[0][0][0][0][0],sizeof(self->acmagnitudebins));
	InitializeJPEGContexts(&self->acremainderbins[0][0][0][0],sizeof(self->acremainderbins));
	InitializeJPEGContexts(&self->acsignbins[0][0][0][0],sizeof(self->acsignbins));
	InitializeJPEGContexts(&self->dcmagnitudebins[0][0][0],sizeof(self->dcmagnitudebins));
	InitializeJPEGContexts(&self->dcremainderbins[0][0][0],sizeof(self->dcremainderbins));
	InitializeJPEGContexts(&self->dcsignbins[0][0][0][0],sizeof(self->dcsignbins));*/

/*	// Allocate memory for each component in a slice.
	for(int i=0;i<self->jpeg.numscancomponents;i++)
	{
		self->blocks[i]=malloc(self->jpeg.horizontalmcus*self->sliceheight*
		self->jpeg.scancomponents[i].component->horizontalfactor*
		self->jpeg.scancomponents[i].component->verticalfactor*
		sizeof(JPEGBlock));

		if(!self->blocks[i]) return JPEGOutOfMemoryError;
	}*/

	return true;
}



//
// Block decoding.
//

// Decoder functions.
/*static void DecodeBlock(JPEGDecompressor *self,int comp,
JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static int DecodeACComponent(JPEGDecompressor *self,int comp,unsigned int k,bool canbezero,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static int DecodeACSign(JPEGDecompressor *self,int comp,unsigned int k,int absvalue,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static int DecodeDCComponent(JPEGDecompressor *self,int comp,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static unsigned int DecodeBinarization(JPEGArithmeticDecoder *decoder,
JPEGContext *magnitudebins,JPEGContext *remainderbins,int maxbits,int cap);

// Coefficient zig-zag ordering functions.
static bool IsFirstRow(unsigned int k);
static bool IsFirstColumn(unsigned int k);
static bool IsFirstRowOrColumn(unsigned int k);
static bool IsSecondRow(unsigned int k);
static bool IsSecondColumn(unsigned int k);

static unsigned int Left(unsigned int k);
static unsigned int Up(unsigned int k);
static unsigned int UpAndLeft(unsigned int k);
static unsigned int Right(unsigned int k);
static unsigned int Down(unsigned int k);

static unsigned int ZigZag(unsigned int row,unsigned int column);
static unsigned int Row(unsigned int k);
static unsigned int Column(unsigned int k);

// Compression primitive functions.
static int Min(int a,int b);
static int Abs(int x);
static int Sign(int x);
static unsigned int Category(unsigned int val);

static int Sum(unsigned int k,const JPEGBlock *block);
static int Average(unsigned int k,
const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);
static int BDR(unsigned int k,const JPEGBlock *current,
const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization);

static const JPEGBlock ZeroBlock={0};

int ReadNextJPEGSlice(JPEGDecompressor *self)
{
	self->currheight=self->sliceheight;
	if(self->finishedrows+self->currheight>=self->jpeg.verticalmcus)
	{
		self->currheight=self->jpeg.verticalmcus-self->finishedrows;
		self->slicesavailable=false;
	}

	for(int i=0;i<self->jpeg.numscancomponents;i++)
	{
		InitializeJPEGArithmeticDecoder(&self->decoder,self->readfunc,self->inputcontext);

		int hblocks=self->jpeg.scancomponents[i].component->horizontalfactor;
		int vblocks=self->jpeg.scancomponents[i].component->verticalfactor;
		int blocksperrow=self->jpeg.horizontalmcus*hblocks;

		const JPEGQuantizationTable *quantization=self->jpeg.scancomponents[i].component->quantizationtable;

		// NOTE: Blocks are processed in cartesian order, not MCU order.
		for(int y=0;y<self->currheight*vblocks;y++)
		for(int x=0;x<self->jpeg.horizontalmcus*hblocks;x++)
		{
			JPEGBlock *currblock=&self->blocks[i][x+y*blocksperrow];

			const JPEGBlock *northblock;
			if(y!=0) northblock=&self->blocks[i][x+(y-1)*blocksperrow];
			else if(self->finishedrows!=0) northblock=&self->blocks[i][x+(self->sliceheight*vblocks-1)*blocksperrow];
			else northblock=NULL;

			const JPEGBlock *westblock;
			if(x!=0) westblock=&self->blocks[i][x-1+y*blocksperrow];
			else westblock=NULL;

			DecodeBlock(self,i,currblock,northblock,westblock,quantization);

			if(JPEGArithmeticDecoderEncounteredEOF(&self->decoder)) return JPEGEndOfStreamError;
		}

		FlushJPEGArithmeticDecoder(&self->decoder);
	}

	self->finishedrows+=self->currheight;

	// Initialize writer state.
	self->mcusavailable=true;
	self->currblock=self->blocks[0];

	self->mcurow=0;
	self->mcucol=0;
	self->mcucomp=0;
	self->mcux=0;
	self->mcuy=0;
	self->mcucoeff=0;

	// TODO: Error handling.
	return JPEGNoError;
}

static void DecodeBlock(JPEGDecompressor *self,int comp,
JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	// Decode End Of Block value to find out how many AC components there are. (5.6.5)

	// Calculate EOB context. (5.6.5.2)
	int average;
	if(!north&&!west) average=0;
	else if(!north) average=Sum(0,west);
	else if(!west) average=Sum(0,north);
	else average=(Sum(0,north)+Sum(0,west)+1)/2;

	int eobcontext=Min(Category(average),12);

	// Decode EOB bits using binary tree. (5.6.5.1)
	unsigned int bitstring=1;
	for(int i=0;i<6;i++)
	{
		bitstring=(bitstring<<1)|NextBitFromJPEGArithmeticDecoder(&self->decoder,
		&self->eobbins[comp][eobcontext][bitstring-1]);
	}
	unsigned int eob=bitstring&0x3f;
	current->eob=eob;

	// Fill out the elided block entries with 0.
	for(unsigned int k=eob+1;k<=63;k++) current->c[k]=0;

	// Decode AC components in decreasing order, if any. (5.6.6)
	for(unsigned int k=eob;k>=1;k--)
	{
		current->c[k]=DecodeACComponent(self,comp,k,k!=eob,current,north,west,quantization);
	}

	// Decode DC component.
	current->c[0]=DecodeDCComponent(self,comp,current,north,west,quantization);
}

static int DecodeACComponent(JPEGDecompressor *self,int comp,unsigned int k,bool canbezero,
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
		// Decode zero/non-zero bit. (5.6.6.1)
		int zerocontext1=Min(Category(val1),2);
		int zerocontext2=Min(Category(val2),5);

		int nonzero=NextBitFromJPEGArithmeticDecoder(&self->decoder,
		&self->zerobins[comp][k-1][zerocontext1][zerocontext2]);

		// If this component is zero, there is no need to decode further parameters.
		if(!nonzero) return 0;
	}

	// This component is not zero. Proceed with decoding absolute value.
	int absvalue;

	// Decode pivot (abs>=2). (5.6.6.2)
	int pivotcontext1=Min(Category(val1),4);
	int pivotcontext2=Min(Category(val2),6);

	int pivot=NextBitFromJPEGArithmeticDecoder(&self->decoder,
	&self->pivotbins[comp][k-1][pivotcontext1][pivotcontext2]);

	if(!pivot)
	{
		// The absolute of this component is not >=2. It must therefore be 1,
		// and there is no need to decode the value.
		absvalue=1;
	}
	else
	{
		// The absolute of this component is >=2. Proceed with decoding
		// the absolute value. (5.6.6.3)
		int val3,n;
		if(IsFirstRow(k)) { val3=Column(k)-1; n=0; }
		else if(IsFirstColumn(k)) { val3=Row(k)-1; n=1; }
		else { val3=Category(k-4); n=2; }

		int magnitudecontext1=Min(Category(val1),8);
		int magnitudecontext2=Min(Category(val2),8);
		int remaindercontext=val3;

		// Decode absolute value.
		absvalue=DecodeBinarization(&self->decoder,
		self->acmagnitudebins[comp][n][magnitudecontext1][magnitudecontext2],
		self->acremainderbins[comp][n][remaindercontext],
		14,9)+2;
	}

	if(DecodeACSign(self,comp,k,absvalue,current,north,west,quantization)) return -absvalue;
	else return absvalue;
}

static int DecodeACSign(JPEGDecompressor *self,int comp,unsigned int k,int absvalue,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	// Decode sign. (5.6.6.4)

	// Calculate sign context, or decode with fixed probability. (5.6.6.4.1)
	int predictedsign;
	if(IsFirstRowOrColumn(k))
	{
		int bdr=BDR(k,current,north,west,quantization);

		if(bdr==0) return NextBitFromJPEGArithmeticDecoder(&self->decoder,&self->fixedcontext);

		predictedsign=(bdr<0);
	}
	else if(k==4)
	{
		int sign1=Sign(north->c[k]);
		int sign2=Sign(west->c[k]);

		if(sign1+sign2==0) return NextBitFromJPEGArithmeticDecoder(&self->decoder,&self->fixedcontext);

		predictedsign=(sign1+sign2<0);
	}
	else if(IsSecondRow(k))
	{
		if(north->c[k]==0) return NextBitFromJPEGArithmeticDecoder(&self->decoder,&self->fixedcontext);

		predictedsign=(north->c[k]<0);
	}
	else if(IsSecondColumn(k))
	{
		if(west->c[k]==0) return NextBitFromJPEGArithmeticDecoder(&self->decoder,&self->fixedcontext);

		predictedsign=(west->c[k]<0);
	}
	else
	{
		return NextBitFromJPEGArithmeticDecoder(&self->decoder,&self->fixedcontext);
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

	return NextBitFromJPEGArithmeticDecoder(&self->decoder,
	&self->acsignbins[comp][n][signcontext1][predictedsign]);
}

static int DecodeDCComponent(JPEGDecompressor *self,int comp,
const JPEGBlock *current,const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	// Decode DC component. (5.6.7)

	// DC prediction. (5.6.7.1)
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

	// Decode DC residual. (5.6.7.3)

	// Decode absolute value. (5.6.7.3.1)
	int absvalue;
	int sum=Sum(0,current);
	int valuecontext=Min(Category(sum),12);

	absvalue=DecodeBinarization(&self->decoder,
	self->dcmagnitudebins[comp][valuecontext],
	self->dcremainderbins[comp][valuecontext],
	15,10);
	if(absvalue==0) return predicted;

	// Decode sign. (5.6.7.3.2)
	// NOTE: Spec says north[0]<0 and west[0]<0.
	if(!north) north=&ZeroBlock;
	if(!west) west=&ZeroBlock;
	int northsign=(north->c[0]<predicted);
	int westsign=(west->c[0]<predicted);
	int predictedsign=(predicted<0);

	int sign=NextBitFromJPEGArithmeticDecoder(&self->decoder,
	&self->dcsignbins[comp][northsign][westsign][predictedsign]);

	if(sign) return predicted-absvalue;
	else return predicted+absvalue;
}

static unsigned int DecodeBinarization(JPEGArithmeticDecoder *decoder,
JPEGContext *magnitudebins,JPEGContext *remainderbins,int maxbits,int cap)
{
	// Decode binarization. (5.6.4, and additional reverse engineering
	// as the spec does not describe the process in sufficient detail.)

	// Decode unary magnitude.
	int ones=0;
	while(ones<maxbits)
	{
		int context=ones;
		if(context>=cap) context=cap-1;

		int unary=NextBitFromJPEGArithmeticDecoder(decoder,&magnitudebins[context]);
		if(unary==1) ones++;
		else break;
	}

	// Decode remainder bits, if any.
	if(ones==0) return 0;
	else if(ones==1) return 1;
	else
	{
		int numbits=ones-1;
		int val=1<<numbits;

		for(int i=numbits-1;i>=0;i--)
		{
			int bit=NextBitFromJPEGArithmeticDecoder(decoder,&remainderbins[i]);
			val|=bit<<i;
		}

		return val;
	}
}

static bool IsFirstRow(unsigned int k) { return Row(k)==0; }
static bool IsFirstColumn(unsigned int k) { return Column(k)==0; }
static bool IsFirstRowOrColumn(unsigned int k) { return IsFirstRow(k)||IsFirstColumn(k); }
static bool IsSecondRow(unsigned int k) { return Row(k)==1; }
static bool IsSecondColumn(unsigned int k) { return Column(k)==1; }

static unsigned int Left(unsigned int k) { return ZigZag(Row(k),Column(k)-1); }
static unsigned int Up(unsigned int k) { return ZigZag(Row(k)-1,Column(k)); }
static unsigned int UpAndLeft(unsigned int k) { return ZigZag(Row(k)-1,Column(k)-1); }
static unsigned int Right(unsigned int k) { return ZigZag(Row(k),Column(k)+1); }
static unsigned int Down(unsigned int k) { return ZigZag(Row(k)+1,Column(k)); }

static unsigned int ZigZag(unsigned int row,unsigned int column)
{
	if(row>=8||column>=8) return 0; // Can't happen.
	static const int table[8][8]=
	{
		{  0, 1, 5, 6,14,15,27,28, },
		{  2, 4, 7,13,16,26,29,42, },
		{  3, 8,12,17,25,30,41,43, },
		{  9,11,18,24,31,40,44,53, },
		{ 10,19,23,32,39,45,52,54, },
		{ 20,22,33,38,46,51,55,60, },
		{ 21,34,37,47,50,56,59,61, },
		{ 35,36,48,49,57,58,62,63, },
	};
	return table[row][column];
}

static unsigned int Row(unsigned int k)
{
	if(k>=64) return 0; // Can't happen.
	static const int table[64]=
	{
		0,0,1,2,1,0,0,1,2,3,4,3,2,1,0,0,
		1,2,3,4,5,6,5,4,3,2,1,0,0,1,2,3,
		4,5,6,7,7,6,5,4,3,2,1,2,3,4,5,6,
		7,7,6,5,4,3,4,5,6,7,7,6,5,6,7,7,
	};
	return table[k];
}

static unsigned int Column(unsigned int k)
{
	if(k>=64) return 0; // Can't happen.
	static const int table[64]=
	{
		0,1,0,0,1,2,3,2,1,0,0,1,2,3,4,5,
		4,3,2,1,0,0,1,2,3,4,5,6,7,6,5,4,
		3,2,1,0,1,2,3,4,5,6,7,7,6,5,4,3,
		2,3,4,5,6,7,7,6,5,4,5,6,7,7,6,7,
	};
	return table[k];
}

static int Min(int a,int b)
{
	if(a<b) return a;
	else return b;
}

static int Abs(int x)
{
	if(x>=0) return x;
	else return -x;
}

static int Sign(int x)
{
	if(x>0) return 1;
	else if(x<0) return -1;
	else return 0;
}

// CAT (5.6.3)
static unsigned int Category(unsigned int val)
{
	if(val==0) return 0;

	unsigned int cat=0;
	if(val&0xffff0000) { val>>=16; cat|=16; }
	if(val&0xff00) { val>>=8; cat|=8; }
	if(val&0xf0) { val>>=4; cat|=4; }
	if(val&0xc) { val>>=2; cat|=2; }
	if(val&0x2) { val>>=1; cat|=1; }
	return cat+1;
}

// SUM (5.6.2.1)
static int Sum(unsigned int k,const JPEGBlock *block)
{
	int sum=0;
	for(unsigned int i=0;i<64;i++)
	{
		if(i!=k && Row(i)>=Row(k) && Column(i)>=Column(k))
		sum+=Abs(block->c[i]);
	}
	return sum;
}

// AVG (5.6.2.2)
// NOTE: This assumes that the expression given for 'sum' is incorrect, and that
// Bw[k] should actually be Bw[x]. Also, the spec does not explicitly mention
// that the DC component never contributes.
static int Average(unsigned int k,
const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	if(k==0||k==1||k==2)
	{
		return (Abs(north->c[k])+Abs(west->c[k])+1)/2;
	}
	else if(IsFirstRow(k))
	{
		return (
			(Abs(north->c[Left(k)])+Abs(west->c[Left(k)]))*quantization->c[Left(k)]/quantization->c[k]+
			Abs(north->c[k])+Abs(west->c[k])+
			2
		)/(2*2);
	}
	else if(IsFirstColumn(k))
	{
		return (
			(Abs(north->c[Up(k)])+Abs(west->c[Up(k)]))*quantization->c[Up(k)]/quantization->c[k]+
			Abs(north->c[k])+Abs(west->c[k])+
			2
		)/(2*2);
	}
	else if(k==4)
	{
		return (
			(Abs(north->c[Up(k)])+Abs(west->c[Up(k)]))*quantization->c[Up(k)]/quantization->c[k]+
			(Abs(north->c[Left(k)])+Abs(west->c[Left(k)]))*quantization->c[Left(k)]/quantization->c[k]+
			Abs(north->c[k])+Abs(west->c[k])+
			3
		)/(2*3);
	}
	else
	{
		return (
			(Abs(north->c[Up(k)])+Abs(west->c[Up(k)]))*quantization->c[Up(k)]/quantization->c[k]+
			(Abs(north->c[Left(k)])+Abs(west->c[Left(k)]))*quantization->c[Left(k)]/quantization->c[k]+
			(Abs(north->c[UpAndLeft(k)])+Abs(west->c[UpAndLeft(k)]))*quantization->c[UpAndLeft(k)]/quantization->c[k]+
			Abs(north->c[k])+Abs(west->c[k])+
			4
		)/(2*4);
	}
}

// BDR (5.6.2.3)
static int BDR(unsigned int k,const JPEGBlock *current,
const JPEGBlock *north,const JPEGBlock *west,
const JPEGQuantizationTable *quantization)
{
	if(IsFirstRow(k))
	{
		return north->c[k]-(north->c[Down(k)]+current->c[Down(k)])*quantization->c[Down(k)]/quantization->c[k];
	}
	else if(IsFirstColumn(k))
	{
		return west->c[k]-(west->c[Right(k)]+current->c[Right(k)])*quantization->c[Right(k)]/quantization->c[k];
	}
	else return 0; // Can't happen.
}



//
// Block encoding.
//

static void PushEncodedValue(JPEGDecompressor *self,JPEGHuffmanTable *table,
int value,unsigned int highbits);
static void PushHuffmanCode(JPEGDecompressor *self,JPEGHuffmanTable *table,unsigned int code);
static void PushBitString(JPEGDecompressor *self,uint32_t bitstring,unsigned int length);

size_t EncodeJPEGBlocksToBuffer(JPEGDecompressor *self,void *bytes,size_t length)
{
	uint8_t	*start=bytes;
	uint8_t	*ptr=bytes;
	uint8_t *end=ptr+length;

	while(ptr<end)
	{
		if(self->needsstuffing)
		{
			// If we need to add a byte of stuffing, do so.
			*ptr++=0x00;
			self->needsstuffing=false;
		}
		else if(self->bitlength>=8)
		{
			// If there are enough buffered bits, output one byte.
			uint8_t byte=self->bitstring>>56LL;
			*ptr++=byte;
			self->bitstring<<=8;
			self->bitlength-=8;

			if(byte==0xff) self->needsstuffing=true;
		}
		else if(self->writerestartmarker)
		{
			// Output the first half of a restart marker. This has to be done
			// separately, to avoid stuffing.
			*ptr++=0xff;

			// Next, push the rest of the marker into the bitstream as usual, as it
			// will not trigger bit stuffing.
			PushBitString(self,0xd0+self->restartmarkerindex,8);

			// Cycle the restart marker indexes, reset the MCU counter, and clear predictors.
			self->restartmarkerindex=(self->restartmarkerindex+1)&7;
			self->mcucounter=0;
			memset(self->predicted,0,sizeof(self->predicted));

			self->writerestartmarker=false;
		}
		else if(self->jpeg.restartinterval && self->mcucounter==self->jpeg.restartinterval &&
		(self->mcusavailable || self->slicesavailable))
		{
			// If it is time for a restart marker, and if we have not reached the very
			// end of the scan, start outputting one.

			// First, pad with ones to a byte border if needed.
			if(self->bitlength)
			{
				int n=8-self->bitlength;
				PushBitString(self,(1<<n)-1,n);
			}

			// Then set a flag to start writing a restart marker. This has to be done
			// separately, to avoid bit stuffing.
			self->writerestartmarker=true;
		}
		else if(self->mcusavailable)
		{
			// If there are still MCUs left to process output either a DC or AC coefficient as appropriate.
			if(self->mcucoeff==0)
			{
				// Output DC coefficient.
				int diff=self->currblock->c[0]-self->predicted[self->mcucomp];

				PushEncodedValue(self,self->jpeg.scancomponents[self->mcucomp].dctable,diff,0);

				self->predicted[self->mcucomp]=self->currblock->c[0];
				self->mcucoeff=1;
			}
			else if(self->mcucoeff>self->currblock->eob && self->currblock->eob!=63)
			{
				// Output EOB marker.
				PushHuffmanCode(self,self->jpeg.scancomponents[self->mcucomp].actable,0x00);

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

				PushEncodedValue(self,self->jpeg.scancomponents[self->mcucomp].actable,val,zeroes);

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
								if(self->mcurow>=self->currheight)
								{
									self->mcusavailable=false;

									if(!self->slicesavailable)
									{
										// If we reached the very end, pad with ones
										// to a byte boundary to finish the stream.
										int n=(-self->bitlength)&7;
										PushBitString(self,(1<<n)-1,n);
									}
								}
							}

							// Count up towards the restart interval.
							self->mcucounter++;
						}
					}
				}

				// Find the new block.
				hblocks=self->jpeg.scancomponents[self->mcucomp].component->horizontalfactor;
				vblocks=self->jpeg.scancomponents[self->mcucomp].component->verticalfactor;
				int blocksperrow=self->jpeg.horizontalmcus*hblocks;

				int blockx=self->mcucol*hblocks+self->mcux;
				int blocky=self->mcurow*vblocks+self->mcuy;

				self->currblock=&self->blocks[self->mcucomp][blockx+blocky*blocksperrow];
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

static void PushEncodedValue(JPEGDecompressor *self,JPEGHuffmanTable *table,
int value,unsigned int highbits)
{
	int category,bitstring;
	if(value>=0)
	{
		category=Category(value);
		int mask=(1<<category)-1;
		bitstring=value&mask;
	}
	else
	{
		category=Category(-value);
		int mask=(1<<category)-1;
		bitstring=(value&mask)-1;
	}

	PushHuffmanCode(self,table,category|(highbits<<4));
	PushBitString(self,bitstring,category);
}


static void PushHuffmanCode(JPEGDecompressor *self,JPEGHuffmanTable *table,unsigned int code)
{
	PushBitString(self,table->codes[code].code,table->codes[code].length);
}

static void PushBitString(JPEGDecompressor *self,uint32_t bitstring,unsigned int length)
{
	self->bitstring|=(uint64_t)bitstring<<64-self->bitlength-length;
	self->bitlength+=length;
}
*/

