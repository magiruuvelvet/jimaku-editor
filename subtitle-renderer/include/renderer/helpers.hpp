#ifndef SUBTITLE_RENDERER_HELPERS_HPP
#define SUBTITLE_RENDERER_HELPERS_HPP

namespace Magick
{
    class Image;
}

unsigned cropDetectionRow(Magick::Image *image, bool fromTop);
unsigned cropDetectionCol(Magick::Image *image, bool fromLeft);

#endif // SUBTITLE_RENDERER_HELPERS_HPP
