#ifndef SUBTITLE_RENDERER_HELPERS_HPP
#define SUBTITLE_RENDERER_HELPERS_HPP

#include <vector>

namespace Magick
{
    class Image;
}

// detects how many rows can be cropped from top
unsigned cropDetectionRow(Magick::Image *image, bool fromTop);

// detects how many columns can be cropped from left
unsigned cropDetectionCol(Magick::Image *image, bool fromLeft);

// counts unique colors in the image and creates a sorted palette from low to high
const std::vector<unsigned char> createPalette(const std::vector<unsigned char> &rgba, unsigned long width, unsigned long height);
const std::vector<unsigned char> createPalette(const unsigned char *rgba, unsigned long width, unsigned long height);

#endif // SUBTITLE_RENDERER_HELPERS_HPP
