CFLAGS=-std=c99 -O3

EXECUTABLES=WiltCompress WiltNewCompress WiltDecompress WiltCompress16 WiltDecompress16

all: $(EXECUTABLES)

clean:
	rm -rf $(EXECUTABLES)

WiltCompress: WiltCompress.c
	$(CC) $(CFLAGS) -o $@ $<

WiltNewCompress: WiltNewCompress.c
	$(CC) $(CFLAGS) -o $@ $<

WiltDecompress: WiltDecompress.c
	$(CC) $(CFLAGS) -o $@ $<

WiltCompress16: WiltCompress16.c
	$(CC) $(CFLAGS) -o $@ $<

WiltDecompress16: WiltDecompress16.c
	$(CC) $(CFLAGS) -o $@ $<
