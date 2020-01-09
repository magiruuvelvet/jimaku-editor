# ToDo List

 - [x] basic srt parser
 - [x] custom srt parser with style hints
   - [x] basic parser
   - [x] validate properties
   - [x] handle defaults based on `text-direction`
 - [ ] rendering of PNG images
   - [x] basic layout and line spacing
   - [x] furigana (horizontal)
   - [x] furigana (vertical)
   - [x] horizontal text
   - [x] vertical text
   - [x] fix text border quality and appearance
   - [x] must reduce color palette to 255 colors using an algorithm (PGS compliance) **!!!**
         otherwise subtitles look extremely ugly once they are in PGS format,
         because the encoder just removes the colors (with a warning) instead of reducing them
   - [ ] horizontal numbers in vertical text
   - [ ] furigana spacing
   - [ ] optimize PNG images to avoid usage of 3rd party software
 - [x] ~~transform PGS encoder into a library and fix memory leaks and possible segfaults (found some problems during my testing)~~ new encoder in use
   - [x] fix seeking problems in ffmpeg (something is corrupt in the PGS file)
   - [x] new encoder warns on complex and invalid image data and print an error message
 - [ ] handle overlapping subtitle frames (log a warning)
 - [x] command line interface
 - [ ] write user guide and documentation
 - [ ] optimize code now that prototyping is done
