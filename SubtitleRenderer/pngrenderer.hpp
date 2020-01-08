/**
 * PNG Renderer
 *
 * Renders a single PNG subtitle image.
 *
 */

#ifndef PNGRENDERER_HPP
#define PNGRENDERER_HPP

#include <string>
#include <vector>

class PNGRenderer
{
public:
    PNGRenderer();
    PNGRenderer(const std::string &text, const std::string &fontFamily = {}, unsigned long fontSize = 48, unsigned long furiganaFontSize = 20);
    ~PNGRenderer() = default;

    enum class TextJustify
    {
        Left,
        Center,
    };

    enum class FuriganaDistance
    {
        None,
        Narrow,
        Far,
        Unchanged,
    };

    // the final image size
    struct size_t
    {
        unsigned width = 0;
        unsigned height = 0;
    };

    // the starting position of the subtitle frame when aligning it inside a PGS frame
    // Furigana are ignored when calculating the starting position, only main characters are respected
    // horizontal == (bottom) left most main char, depending on the text justification (x,y = glyph bottom left)
    // vertical == top right most main char (x,y == glyph top right)
    struct pos_t
    {
        bool vertical = false;
        unsigned x = 0;
        unsigned y = 0;
    };

    inline void setVertical(bool vertical)
    {
        _vertical = vertical;
    }

    inline void setText(const std::string &text)
    {
        _text = text;
    }

    inline void setFontFamily(const std::string &fontFamily)
    {
        _fontFamily = fontFamily;
    }

    inline void setFontSize(unsigned long fontSize)
    {
        _fontSize = fontSize;
    }

    inline void setFontColor(const std::string &fontColor)
    {
        _fontColor = fontColor;
    }

    inline void setFuriganaFontSize(unsigned long furiganaFontSize)
    {
        _furiganaFontSize = furiganaFontSize;
    }

    inline void setFuriganaFontColor(const std::string &furiganaFontColor)
    {
        _furiganaFontColor = furiganaFontColor;
    }

    inline void setLineSpaceReduction(int lineSpaceReduction)
    {
        _lineSpaceReduction = lineSpaceReduction;
    }

    inline void setFuriganaLineSpaceReduction(int furiganaLineSpaceReduction)
    {
        _furiganaLineSpaceReduction = furiganaLineSpaceReduction;
    }

    inline void setTextJustify(TextJustify textJustify)
    {
        _textJustify = textJustify;
    }

    inline void setTextJustify(const std::string &textJustify)
    {
        if (textJustify == "left")
        {
            _textJustify = TextJustify::Left;
        }
        else if (textJustify == "center")
        {
            _textJustify = TextJustify::Center;
        }
        else
        {
            _textJustify = TextJustify::Center;
        }
    }

    inline void setFuriganaDistance(FuriganaDistance furiganaDistance)
    {
        _furiganaDistance = furiganaDistance;
    }

    inline void setFuriganaDistance(const std::string &furiganaDistance)
    {
        if (furiganaDistance == "none")
        {
            _furiganaDistance = FuriganaDistance::None;
        }
        else if (furiganaDistance == "narrow")
        {
            _furiganaDistance = FuriganaDistance::Narrow;
        }
        else if (furiganaDistance == "far")
        {
            _furiganaDistance = FuriganaDistance::Far;
        }
        else
        {
            _furiganaDistance = FuriganaDistance::Unchanged;
        }
    }

    inline void setBorderColor(const std::string &borderColor)
    {
        _borderColor = borderColor;
    }

    inline void setBorderSize(unsigned long borderSize)
    {
        _borderSize = borderSize;
    }

    inline void setFuriganaBorderSize(unsigned long furiganaBorderSize)
    {
        _furiganaBorderSize = furiganaBorderSize;
    }

    const std::vector<char> render(size_t *size = nullptr, pos_t *pos  = nullptr, unsigned long *color_count = nullptr) const;

private:
    bool _vertical = false;
    std::string _text;
    std::string _fontFamily;
    unsigned long _fontSize = 46;
    std::string _fontColor = "#f1f1f1";
    unsigned long _furiganaFontSize = 20;
    std::string _furiganaFontColor = "#f1f1f1";
    int _lineSpaceReduction = 0;
    int _furiganaLineSpaceReduction = 0;
    TextJustify _textJustify = TextJustify::Center;
    FuriganaDistance _furiganaDistance = FuriganaDistance::Unchanged;
    std::string _borderColor = "#191919";
    unsigned long _borderSize = 3;
    unsigned long _furiganaBorderSize = 2;
    // TODO: properties
    double _gaussianBlurRadius = 10;
    double _gaussianBlurSigma = 0.8;
    unsigned _colorLimit = 64;
};

#endif // PNGRENDERER_HPP
