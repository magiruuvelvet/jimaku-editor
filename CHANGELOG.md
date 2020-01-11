# Changelog

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
