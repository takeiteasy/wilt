# Wilt #

Wilt is a compression algorithm designed to be very simple to implement, but still competitive with popular compression algorithms. Wilt usually beats Deflate/gzip, and can beat bzip2, while taking around a single page of C code to implement (it will however usually lose to LZMA). It is especially good at small files, and files with aligned data.

Since it is very simple, it is also simple to adapt for specific uses. One variant provided is Wilt-16, which works on 16-bit values instead of bytes, and was developed for use on the Nintendo DS.

For more information, refer to the wiki pages and the source code.

* [WiltBenchmarks](https://bitbucket.org/WAHa_06x36/wilt/wiki/WiltBenchmarks) Benchmarks for Wilt.
* [WiltStreamFormat](https://bitbucket.org/WAHa_06x36/wilt/wiki/WiltStreamFormat) Description of the data stream format for the Wilt compressor.
* [WiltVariants](https://bitbucket.org/WAHa_06x36/wilt/wiki/WiltVariants) Variants of the Wilt algorithm

## License ##

This code is released into the public domain with no warranties. If that is not suitable, it is also available under the [CC0 license](http://creativecommons.org/publicdomain/zero/1.0/).