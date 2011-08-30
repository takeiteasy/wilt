CFLAGS=-std=c99 -g

LDFLAGS=
LIBS=-lm

CC=gcc
LD=gcc

ALL_C_FILES=Compressor.c \
            Decompressor.c \
            JPEG.c \
            SimpleCompress.c \
            SimpleDecompress.c \
            ../RangeCoder/RangeDecoder.c \
            ../RangeCoder/RangeEncoder.c

SIMPLECOMPRESS_C_FILES=Compressor.c \
                       JPEG.c \
                       SimpleCompress.c \
                       ../RangeCoder/RangeEncoder.c

SIMPLEDECOMPRESS_C_FILES=Decompressor.c \
                         JPEG.c \
                         SimpleDecompress.c \
                         ../RangeCoder/RangeDecoder.c

ALL_OBJS=$(ALL_C_FILES:.c=.o)
SIMPLECOMPRESS_OBJS=$(SIMPLECOMPRESS_C_FILES:.c=.o)
SIMPLEDECOMPRESS_OBJS=$(SIMPLEDECOMPRESS_C_FILES:.c=.o)

all: SimpleCompress SimpleDecompress

SimpleCompress: $(SIMPLECOMPRESS_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

SimpleDecompress: $(SIMPLEDECOMPRESS_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(ALL_OBJS) SimpleCompress SimpleDecompress

depend:
	$(CC) -MM $(CFLAGS) $(ALL_C_FILES) >Dependencies.tmp
	sed 's/.*: *\([^ ]*\)\.\([^ ]*\)/\1.o: \1.\2/g' <Dependencies.tmp >Dependencies
	rm Dependencies.tmp

include Dependencies

