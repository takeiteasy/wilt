#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

#include <stdint.h>
#include <stdbool.h>



// Zig-zag ordering functions.

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




// Integer operations.

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



// Compression context primitives.

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

#endif

