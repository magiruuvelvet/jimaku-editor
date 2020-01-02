# 字幕エーディタ (Subtitle Editor)

An experimental work-in-progress subtitle editor with
**first class** Japanese language support.

### Planned Features

 - Furigana

 - Vertical writing (top to bottom)

 - Rendering the finished subtitle to PGS (Presentation Graphic Stream) to
   overcome the limitations of text-based subtitles when it comes to the
   above features. (SRT/ASS/etc. don't support complex text layouts)

   There is already a working open source PGS encoder I'm planning to use
   for this project. I tested it and it works great and the PGS files are
   compatible with ffmpeg and ffmpeg-based media players :)

   --> [drouarb/PGSEncoder](https://github.com/drouarb/PGSEncoder)

 - Parse SRT files for Japanese subtitles (many of them can be found on the Internet)\
   Many existing subtitles can be made pretty and probably styled that way, without
   starting over from scratch.


## Why?

Because I want proper Japanese subtitles like they are shown on Netflix
or how they are found on Japanese Blu-Ray discs. Since the open source
community seems not to be very interested in supporting anything other
than European/American languages for subtitles, I decided to give it a
try myself. This editor might work for Chinese in the future too.


### Example PGS subtitle in MPV

Example showing vertical writing with furigana on the right side.

![PGS Example in MPV](.readme/vertical-text-with-furigana-example.png)

[Download](.readme/example.sup) and try yourself.

