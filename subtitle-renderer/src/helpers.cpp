#include "helpers.hpp"

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
