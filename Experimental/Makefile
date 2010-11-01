CFLAGS=-std=c99 -g

EXECUTABLES=WiltCompress WiltDecompress \
	Wilt16Compress Wilt16Decompress \
	Wilt2Compress Wilt2Decompress

all: $(EXECUTABLES)

clean:
	rm -f $(EXECUTABLES) *.o RangeCoder/*.o

WiltCompress: WiltCompress.o RangeCoder/RangeEncoder.o
	$(CC) $(CFLAGS) -o $@ $^

WiltDecompress: WiltDecompress.o RangeCoder/RangeDecoder.o
	$(CC) $(CFLAGS) -o $@ $^

Wilt16Compress: Wilt16Compress.o RangeCoder/RangeEncoder.o
	$(CC) $(CFLAGS) -o $@ $^

Wilt16Decompress: Wilt16Decompress.o RangeCoder/RangeDecoder.o
	$(CC) $(CFLAGS) -o $@ $^

Wilt2Compress: Wilt2Compress.o RangeCoder/RangeEncoder.o
	$(CC) $(CFLAGS) -o $@ $^

Wilt2Decompress: Wilt2Decompress.o RangeCoder/RangeDecoder.o
	$(CC) $(CFLAGS) -o $@ $^

Wilt16Compress.o: Wilt16Compress.c RangeCoder/RangeEncoder.h
Wilt16Decompress.o: Wilt16Decompress.c RangeCoder/RangeDecoder.h
Wilt2Compress.o: Wilt2Compress.c RangeCoder/RangeEncoder.h
Wilt2Decompress.o: Wilt2Decompress.c RangeCoder/RangeDecoder.h
WiltCompress.o: WiltCompress.c RangeCoder/RangeEncoder.h
WiltDecompress.o: WiltDecompress.c RangeCoder/RangeDecoder.h

RadixRangeDecoder.o: RangeCoder/RadixRangeDecoder.c \
  RangeCoder/RadixRangeDecoder.h RangeCoder/DecodeImplementations.h
RadixRangeEncoder.o: RangeCoder/RadixRangeEncoder.c \
  RangeCoder/RadixRangeEncoder.h RangeCoder/EncodeImplementations.h
RangeDecoder.o: RangeCoder/RangeDecoder.c RangeCoder/RangeDecoder.h \
  RangeCoder/DecodeImplementations.h
RangeEncoder.o: RangeCoder/RangeEncoder.c RangeCoder/RangeEncoder.h \
  RangeCoder/EncodeImplementations.h
Test.o: RangeCoder/Test.c
Test1.o: RangeCoder/Test1.c RangeCoder/RangeEncoder.h \
  RangeCoder/RangeDecoder.h RangeCoder/RadixRangeEncoder.h \
  RangeCoder/RadixRangeDecoder.h
Test2.o: RangeCoder/Test2.c RangeCoder/RangeEncoder.h \
  RangeCoder/RangeDecoder.h
Test3.o: RangeCoder/Test3.c RangeCoder/RangeEncoder.h \
  RangeCoder/RangeDecoder.h
Test4.o: RangeCoder/Test4.c RangeCoder/RangeEncoder.h \
  RangeCoder/RangeDecoder.h
Test5.o: RangeCoder/Test5.c RangeCoder/RangeEncoder.h \
  RangeCoder/RangeDecoder.h
