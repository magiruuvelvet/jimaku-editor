# 字幕エーディタ (Subtitle Editor)

An experimental work-in-progress subtitle editor with
**first class** Japanese language support.

Examples and rendering tests available at [jimaku-editor-examples](https://github.com/magiruuvelvet/jimaku-editor-examples)

## Disclaimer

This subtitle editor is designed only for the Japanese language
(Chinese works too, because font glyphs have the same width).
Rendering other writing systems (like latin, cyrillic) is undefined
and untested and may produce funny rendering results. If you plan
to render non-Japanese text into PGS with this you are on your own,
as I don't provide support for this. Good luck.

## Features

 - Furigana

 - Vertical writing (top to bottom)

 - Boutens (just `・` rendered as Furigana or another appropriate symbol)

 - Rendering the finished subtitle to PGS (Presentation Graphic Stream) to
   overcome the limitations of text-based subtitles when it comes to the
   above features. (SRT/ASS/etc. don't support complex text layouts)

 - CLI Interface

#### ToDo Features

 - Horizontal numbers in vertical text

 - **LONG TERM**: GUI?

## How does it work?

The first stage of the project is to get a command line renderer done.
The idea is to take the text, together with **styling and layout hints**,
as input and the application renders PNG images out of it, which can be
used to feed the PGS encoder to produce the final picture-based subtitle
which looks the same on every device and player.

The hard part will be the rendering of Furigana and vertical text.
For vertical text some font glyphs must be rotated 90 degree clockwise.
If you known Japanese than you know what I'm talking about.

For this I need to create a new subtitle format which can store several
styling and layout hints. To keep it simple I'm just extending the SRT
format with an additional line after the timestamp.

#### Example

```plain
2
00:00:16,599 --> 00:00:18,935 
# text-direction=horizontal
# text-alignment=center
# margin-overwrite=90
# font-size=42
（{宮内|みやうち}れんげ）おおーっ！
```

My idea is to have **global hints** on the top of the subtitle file which
applies for all lines and support overwriting hints for each line by
adding the hints below the timestamp.


## Styling and Layout Hints

*This list is a work in progress!!*

All numeric values can be either negative or positive, unless otherwise mentioned.

 - `text-direction`

   Possible values: `horizontal` (default), `vertical`

 - `text-alignment`

   The alignment of the text on the screen.

   Possible values for `horizontal`: `left`, `center` (default), `right`\
   Possible values for `vertical`: `right` (default), `left` (example image below is `right` aligned)

 - `text-justify`

   The text justification within the subtitle frame once rendered.

   Possible values for `horizontal`: `left`, `center` (default)\
   Possible values for `vertical`: `top` (default)

 - `margin-overwrite` **(overwrite only)**

   This property instructs the renderer how much margin from out-to-in
   must be applied for the subtitle frame.

   When the direction is horizontal this adjusts the bottom margin.\
   When the direction is vertical this adjusts the side margin (left/right),
   depending on the text alignment.

   **Value can not be negative!**

 - `margin-bottom`

   Default bottom margin. Default is 150.

   **Value can not be negative!**

 - `margin-side`

   Default side margin. Default is 130.

   **Value can not be negative!**

 - `margin-top`

   For vertical text only. Default top margin.
   Default is 90.

   **Value can not be negative!**

 - `font-family`

   The font family the renderer should use to produce PNG images.

   The default font is `TakaoPGothic`.

 - `font-size`

   The size of the font in `pt`. Default is 46.

   **Value can not be negative!**

 - `font-color`

   The color of the font in HTML format with leading `#`.\
   Default is `#f1f1f1`.

   Recommended colors: `#e6e6e6`, `#d9d9d9`

 - `horizontal-numbers`

   This property only applies when `vertical` text direction is used.
   This instructs the renderer to place numbers horizontally on the
   same line instead of placing them from top to bottom.

   Possible values: `true` (default), `false`

 - `furigana-spacing`

   The spacing between each Kana. Defaults to the font glyph spacing.
   The value for the font default is `font`. Otherwise a number in pixel.

   Negative values decrease the spacing.

 - `furigana-distance`

   The distance between the Kanji and Furigana. This option is interesting together with
   the `line-space-reduction` option to adjust the Furigana distance on bad fonts.
   By default the distance is calculated to be directly placed to the Kanji, which works
   for most fonts. For fonts with huge glyph heights 3 possible values are offered to place
   the Furigana closer to the Kanji.

   Possible values: `none`, `narrow`, `far`, `unchanged` (default, use font defaults)

 - `furigana-font-size`

   The size of the font in `pt` for Furigana. Default is 20.

   **Value can not be negative!**

 - `furigana-font-color`

   The color of the Furigana font in HTML format with leading `#`.\
   Default is `#f1f1f1`.

 - `line-space-reduction`, `furigana-line-space-reduction`

   A value in pixel how much the spacing between lines in multi-line
   subtitle frames should be reduced. This may depend on the chosen
   font for the subtitle. Some fonts may have too much glyph height,
   which causes a huge amount on line spacing being rendered between lines.

   A seperate option for Furigana exists too.

   The default value is 0 which means don't change the line spacing and use the font default.

   Negative values increase the line spacing. Useful when the line spacing is too compact.

 - `border-color`

   The color of the text border in HTML format with leading `#`.\
   Default is `#191919`.

 - `border-size`

   The border size in pixel for the main text.\
   Default is 3.

   **Value can not be negative!**

 - `furigana-border-size`

   The border size in pixel for the Furigana.\
   Default is 2.

   **Value can not be negative!**

 - `blur-radius`

   Gaussian blur radius. Value can have decimal places.

   Default is 10

 - `blur-sigma`

   Gaussian blur sigma. Value can have decimal places.

   Default is 0.5

 - `color-limit`

   The maximum number of colors in the subtitle image.
   The default is 64 colors. The maximum count of unique
   colors a PGS subtitle palette segment can have is 255.
   The encoder throws an error if more than 255 colors
   were calculated during processing the pixel data.


## Furigana

To instruct the renderer about Furigana I'm using the following format:

`{宮内|みやうち}れんげ`

The original text and Furigana are placed inside `{}`. The `|` (pipe) symbol
acts as separator. That way no complicated implementation must be done and
using of a library like ICU is not required. This makes is also possible for
subtitle writers to exactly place the Furigana were it belongs too.

This syntax is also already known to avoid confusing people.

The decision where the Furigana should be placed is automatic. The rules
are as follows:

 - horizontal first/top line == above Kanji
 - horizontal second/bottom line == below Kanji
 - vertical first/right line == right-side of Kanji
 - vertical second/left line == left-side of Kanji

Furigana are always "center" aligned for the first version of this project.
In the future I may add extra alignment hints for Furigana.

**Attention**: Furigana between lines which is not the first or the last line
are silently removed from the rendering result. If you are a subtitle writer,
take that in mind. In general, subtitles should not have more than 2 lines,
except when the first line is only the speaker name. The **hard limit** is 3 lines.
While the renderer just takes it, the rendering result may be undefined for more
than 3 lines.


## Why?

Because I want proper Japanese subtitles like they are shown on Netflix
or how they are found on Japanese Blu-Ray discs. Since the open source
community seems not to be very interested in supporting anything other
than European/American languages for subtitles, I decided to give it a
try myself. This editor might work for Chinese in the future too.

Recommended article for interested people:
[Implementing Japanese subtitles on Netflix](https://medium.com/netflix-techblog/implementing-japanese-subtitles-on-netflix-c165fbe61989)
