CFLAGS=-std=c99 -g

LDFLAGS=
LIBS=-lm

CC=gcc
LD=gcc

ALL_C_FILES=Compressor.c \
            Huffman.c \
            JPEG.c \
            SimpleCompressor.c \
            ../RangeCoder/RangeEncoder.c

SIMPLECOMPRESSOR_C_FILES=Compressor.c \
                         Huffman.c \
                         JPEG.c \
                         SimpleCompressor.c \
                         ../RangeCoder/RangeEncoder.c

ALL_OBJS=$(ALL_C_FILES:.c=.o)
SIMPLECOMPRESSOR_OBJS=$(SIMPLECOMPRESSOR_C_FILES:.c=.o)

all: SimpleCompressor

SimpleCompressor: $(SIMPLECOMPRESSOR_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(ALL_OBJS) SimpleCompressor

depend:
	$(CC) -MM $(CFLAGS) $(ALL_C_FILES) >Dependencies.tmp
	sed 's/.*: *\([^ ]*\)\.\([^ ]*\)/\1.o: \1.\2/g' <Dependencies.tmp >Dependencies
	rm Dependencies.tmp

include Dependencies

