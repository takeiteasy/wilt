CFLAGS=-std=c99 -g

all: Test

clean:
	rm -f Test *.o

Test: Test.o RangeEncoder.o RangeDecoder.o
	$(CC) $(CFLAGS) -o $@ $^

RangeEncoder.o: RangeEncoder.c RangeEncoder.h
RangeDecoder.o: RangeDecoder.c RangeDecoder.h
Test.o: Test.c RangeEncoder.h RangeDecoder.h
