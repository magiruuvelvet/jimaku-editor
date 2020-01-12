# Changelog

## 0.8.5-beta

**PGS Encoder**
- set unused framerate bit to 0x10 (was 0x40)

**Renderer**
- set default color limit to 40 colors
- create color palette for 8-bit indexed image
- resulting png files are now in 8-bit colormap format
- disable png zlib compression for speed

## 0.8.4-beta

**PGS Encoder**
 - fix exit status codes to report errors correctly
 - remove decoding timestamps, in practice those times are always zero

## 0.8.3-beta

 - add warning when image size is too big (has mismatches)
 - add user command support using reproc++
 - add protection against dangerous commands
 - minor ux improvement in cli interface

## 0.8.2-beta

 - fix integer underflow when calculating the subtitle position

## 0.8.1-beta

 - reduce image size to a bare minimum, this allows more
   opaque data inside a single PGS subtitle frame

## 0.8-beta

 - initial release with command line interface
 - already functional for many use cases
 - rendered PNGs must be optimized with 3rd party software
   like **optipng**, **pngcrush** or **pngquant** to make
   them compatible with the PGS format, the encoder warns
   about complex image data
