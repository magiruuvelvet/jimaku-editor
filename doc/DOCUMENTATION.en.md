# Documentation

**work in progress**

## Index

1. About this application
2. Subtitle Format
3. Rendering Subtiltes\
 3.1. Color Palette and Image Size
4. External Commands\
 4.1. Placeholders\
 4.2. Useful post processing commands

# 1. About this application

This application aims to be a high quality **open-source** picture-based
subtitle renderer with first class Japanese language support.

Many (all?) text-based subtitle formats, which are actually implemented in
open-source media players don't support complex text layouts. They are limited
to horizontal writing from left to right and top to bottom. This is a problem
for Japanese subtitles, because you can't correctly display Furigana or make
use of vertical writing from top to bottom and right to left. Having this
limitations forces subtitle writers to make intentionally bad subtitles.

I personally suffered from this limitations too since I started watching
movies and shows in Japanese with Japanese subtitles enabled on Netflix.
On Netflix, Japanese subtiltes are amazing. I wanted a similar experience
for my local collection too so I started to research what options are
available for picture-based subtitles in open-source media players and
found the format used on Blu-ray discs to be very suitable.

And that's how this software was born :)

For an interesting read check out this article from the Netflix Tech Blog.\
[Implementing Japanese subtitles on Netflix](https://medium.com/netflix-techblog/implementing-japanese-subtitles-on-netflix-c165fbe61989)

# 2. Subtitle Format

TODO...

# 3. Rendering Subtiltes

TODO...

## 3.1. Color Palette and Image Size

The less colors the image has, the better are the encoding results
in the PGS subtitle. The maximum amount of allowed colors for a
single PGS subtitle frame are 255 colors. The encoder gives you an
error if you exceed the maximum allowed color count. Having a smaller
color palette allows you to have bigger (width, height) images in
the PGS subtitle frame.

Color Palette + Pixel Data in YCbCrA format must not exceed 65535 bytes
per image.

**Hint:** If you would theoretically remove the size check of
65535 bytes and just force the image into the PGS subtitle,
than most PGS decoders will only show you the subtitles up to that
one frame and than die with an decoding error. All subtitles after
the oversized frame are not shown anymore. This is the case for
ffmpeg-based media players. Bypassing this limitations also causes
seeking issues in most players.

# 4. External Commands

The renderer supports executing external commands on every rendered
PNG file to easily integrate your custom post processing steps.
By default no command is ran. They must be explicitly specified so
the renderer knows about them.

The command is specified using the `--command "command arg1 arg2"`
argument. You can not use spaces in arguments as this is not a shell
interpreter. If you have more complex commands wrap them inside a
script and specify that one instead.

On UNIX-like systems everything which your shell can execute, can be
executed by this application. Shebang is supported.

On Windows the `CreateProcess` syscall is used.

## 4.1. Placeholders

 - `%f`\
   The absolute path to the current PNG file. The path can contain
   spaces. The library which is used to execute operating system
   commands handles this correctly already. If this placeholder is
   omitted from the command, it is appended as last argument automatically.

More placeholders may be added in a future release.

## 4.2. Useful post processing commands

 - `optipng -strip all -zc1 -zm1 -zs0 -f0`\
   Reduce the color palette and remove useless information
   with very minimal zlib compression. Note that zlib compression
   is totally useless when you only feed the images to the PGS
   encoder, which in turn transforms the images into a YCbCrA
   color space with no compression at all.

 - `pngquant -f --strip --ext .png`\
   Quantize and reduce the color palette to an absolute minimum
   possible and remove useless information. Slower than `optipng`,
   but produces better images for use with the PGS encoder.

