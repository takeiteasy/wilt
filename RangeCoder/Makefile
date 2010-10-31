CFLAGS=-std=c99 -g

all: Test

clean:
	rm -f Test *.o

Test: Test.o Test1.o Test2.o Test3.o \
      RangeEncoder.o RangeDecoder.o \
      RadixRangeEncoder.o RadixRangeDecoder.o
	$(CC) $(CFLAGS) -o $@ $^

RadixRangeDecoder.o: RadixRangeDecoder.c RadixRangeDecoder.h \
  DecodeImplementations.h
RadixRangeEncoder.o: RadixRangeEncoder.c RadixRangeEncoder.h \
  EncodeImplementations.h
RangeDecoder.o: RangeDecoder.c RangeDecoder.h DecodeImplementations.h
RangeEncoder.o: RangeEncoder.c RangeEncoder.h EncodeImplementations.h
Test.o: Test.c
Test1.o: Test1.c RangeEncoder.h RangeDecoder.h RadixRangeEncoder.h \
  RadixRangeDecoder.h
Test2.o: Test2.c RangeEncoder.h RangeDecoder.h
Test3.o: Test3.c RangeEncoder.h RangeDecoder.h
