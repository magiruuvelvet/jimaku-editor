#include "helpers.hpp"

#include <algorithm>

#define MAGICKCORE_QUANTUM_DEPTH 8
#define MAGICKCORE_HDRI_ENABLE 1
#include <Magick++.h>

unsigned cropDetectionRow(Magick::Image *image, bool fromTop)
{
    unsigned emptyRows = 0;

    if (fromTop)
    {
        for (auto row = 0U; row < image->size().height(); ++row)
        {
            bool quit = false;

            // image width
            const auto quant = image->getConstPixels(0, row, image->size().width(), 1);

            // pixel count (RGBA => width*4)
            for (auto col = 0U; col < image->size().width() * 4; ++col)
            {
                if (quant[col] > 0)
                {
                    quit = true;
                    break;
                }
            }

            if (quit)
            {
                break;
            }

            ++emptyRows;
        }
    }
    else
    {
        for (auto row = image->size().height(); row != 0; --row)
        {
            bool quit = false;

            // image width
            const auto quant = image->getConstPixels(0, row, image->size().width(), 1);

            // pixel count (RGBA => width*4)
            for (auto col = 0U; col < image->size().width() * 4; ++col)
            {
                if (quant[col] > 0)
                {
                    quit = true;
                    break;
                }
            }

            if (quit)
            {
                break;
            }

            ++emptyRows;
        }
    }

    return emptyRows;
}

unsigned cropDetectionCol(Magick::Image *image, bool fromLeft)
{
    unsigned emptyCols = 0;

    if (fromLeft)
    {
        for (auto col = 0U; col < image->size().width(); ++col)
        {
            bool quit = false;

            // image width
            const auto quant = image->getConstPixels(col, 0, 1, image->size().height());

            // pixel count (RGBA => height*4)
            for (auto row = 0U; row < image->size().height() * 4; ++row)
            {
                if (quant[row] > 0)
                {
                    quit = true;
                    break;
                }
            }

            if (quit)
            {
                break;
            }

            ++emptyCols;
        }
    }
    else
    {
        for (auto col = image->size().width(); col != 0; --col)
        {
            bool quit = false;

            // image width
            const auto quant = image->getConstPixels(col, 0, 1, image->size().height());

            // pixel count (RGBA => height*4)
            for (auto row = 0U; row < image->size().height() * 4; ++row)
            {
                if (quant[row] > 0)
                {
                    quit = true;
                    break;
                }
            }

            if (quit)
            {
                break;
            }

            ++emptyCols;
        }
    }

    return emptyCols;
}

const std::vector<unsigned char> createPalette(const std::vector<unsigned char> &rgba, unsigned long width, unsigned long height)
{
    std::vector<std::uint32_t> colors;

    // scan all pixels
    for (auto i = 0U; i < width * height * 4; i += 4)
    {
        // push entire pixel into palette
        std::uint32_t pixel = (
            ((std::uint8_t) rgba[i] << 24) |
            ((std::uint8_t) rgba[i + 1] << 16) |
            ((std::uint8_t) rgba[i + 2] << 8) |
            ((std::uint8_t) rgba[i + 3])
        );
        colors.emplace_back(pixel);
    }

    // sort colors from low to high
    std::sort(colors.begin(), colors.end(), [](auto&& left, auto&& right) {
        return left < right;
    });

    // remove duplicates
    auto last = std::unique(colors.begin(), colors.end());
    colors.erase(last, colors.end());

    // create palette
    std::vector<unsigned char> palette;

    for (auto&& color : colors)
    {
        auto r = ((color >> 24) & 0xff);
        auto g = ((color >> 16) & 0xff);
        auto b = ((color >> 8) & 0xff);
        auto a = color & 0xff;

        palette.emplace_back(r);
        palette.emplace_back(g);
        palette.emplace_back(b);
        palette.emplace_back(a);
    }

    return palette;
}

const std::vector<unsigned char> createPalette(const unsigned char *rgba, unsigned long width, unsigned long height)
{
    const std::vector<unsigned char> data(rgba, rgba + (width * height * 4));
    return createPalette(data, width, height);
}
