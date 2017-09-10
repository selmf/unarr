# (lib)unarr

**(lib)unarr** is a decompression library for RAR, TAR, ZIP and 7z* archives.

It was forked from **unarr**, which originated as a port of the RAR extraction
features from The Unarchiver project required for extracting images from comic
book archives. [Zeniko](https://github.com/zeniko/) wrote unarr as an
alternative to libarchive which didn't have support for parsing filters or
solid compression at the time.

While (lib)unarr was started with the intent of providing unarr with a proper
cmake based build system suitable for packaging and cross-platform development,
it's focus has now been extended to provide code maintenance and to continue the
development of unarr, which no longer is maintained.

## Getting started

### Building from source

#### Dependencies

(lib)unarr can take advantage of the following libraries if they are present:

* bzip2
* xz / libLZMA
* zlib

More information on what library is used for which purpose can be found in the
description for embedded builds.

#### Cmake

>mkdir build  
>cd build  
>cmake ..  
>make

... as a static library

>cmake .. -DBUILD_SHARED_LIBS=OFF

Install

>make install  

#### Embedded build

Make sure your compiler is C99 compatible, grab the source code, copy it into your project and adjust your build system accordingly.

You can define the following symbols to take advantage of third party libraries:

| Symbol            | Required header | Required for (format/method)|
|-------------------|:---------------:|:----------------------------|
|HAVE_ZLIB          |     zlib.h      |  faster CRC-32 and Deflate  |
|HAVE_BZIP2         |     bzlib.h     |    ZIP / Bzip2              |
|HAVE_LIBLZMA       |     lzma.h      |    ZIP / LZMA, XZ(LZMA2)    |
|HAVE_7Z            |     7z.h        |    7Z* / LZMA, LZMA2, BCJ   |
|_7ZIP_PPMD_SUPPORT |                 |    7Z* / PPMd               |

Make sure the required headers are present in the include path.

## Usage

### Examples

Check [unarr.h](unarr.h) and [unarr-test](test/main.c) to get a general feel
for the api and usage.

## Limitations

Unarr was written for comic book archives, so it currently doesn't support:

* password protected archives
* self extracting archives
* split archives

### 7z support

7z support is currently broken. This is due to two problems:

1. The version of the LZMA SDK used in the 7z extraction code is outdated and
no longer compatible with the LZMA code used in the other formats
2. The ANSI-C based 7z extraction code provided by the LZMA SDK has a known
performance issue that limits it's usefullness for large files with solid
compression (see https://github.com/zeniko/unarr/issues/4).

Fixing these problems requires a partial rewrite of the code involved.
