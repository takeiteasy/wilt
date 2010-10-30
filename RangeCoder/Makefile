CFLAGS=-std=c99 -g

all: Test

clean:
	rm -f Test *.o

Test: Test.o RangeEncoder.o RangeDecoder.o RadixRangeEncoder.o RadixRangeDecoder.o
	$(CC) $(CFLAGS) -o $@ $^

RadixRangeDecoder.o: RadixRangeDecoder.c RadixRangeDecoder.h
RadixRangeEncoder.o: RadixRangeEncoder.c RadixRangeEncoder.h
RangeDecoder.o: RangeDecoder.c RangeDecoder.h
RangeEncoder.o: RangeEncoder.c RangeEncoder.h
Test.o: Test.c RangeEncoder.h RangeDecoder.h
