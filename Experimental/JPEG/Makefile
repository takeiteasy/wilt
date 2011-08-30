CFLAGS=-std=c99 -g

LDFLAGS=
LIBS=-lm

CC=gcc
LD=gcc

ALL_C_FILES=Compressor.c \
            Decompressor.c \
            Huffman.c \
            JPEG.c \
            SimpleCompressor.c \
            ../RangeCoder/RangeDecoder.c \
            ../RangeCoder/RangeEncoder.c

SIMPLECOMPRESSOR_C_FILES=Compressor.c \
                         Huffman.c \
                         JPEG.c \
                         SimpleCompressor.c \
                         ../RangeCoder/RangeEncoder.c

SIMPLEDECOMPRESSOR_C_FILES=Decompressor.c \
                           Huffman.c \
                           JPEG.c \
                           SimpleDecompressor.c \
                           ../RangeCoder/RangeDecoder.c

ALL_OBJS=$(ALL_C_FILES:.c=.o)
SIMPLECOMPRESSOR_OBJS=$(SIMPLECOMPRESSOR_C_FILES:.c=.o)
SIMPLEDECOMPRESSOR_OBJS=$(SIMPLEDECOMPRESSOR_C_FILES:.c=.o)

all: SimpleCompressor SimpleDecompressor

SimpleCompressor: $(SIMPLECOMPRESSOR_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

SimpleDecompressor: $(SIMPLEDECOMPRESSOR_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(ALL_OBJS) SimpleCompressor SimpleDecompressor

depend:
	$(CC) -MM $(CFLAGS) $(ALL_C_FILES) >Dependencies.tmp
	sed 's/.*: *\([^ ]*\)\.\([^ ]*\)/\1.o: \1.\2/g' <Dependencies.tmp >Dependencies
	rm Dependencies.tmp

include Dependencies

