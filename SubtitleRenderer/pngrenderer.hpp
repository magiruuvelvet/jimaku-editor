/**
 * PNG Renderer
 *
 * Renders a single PNG image.
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
    PNGRenderer(const std::string &text, const std::string &fontFamily = {}, int fontSize = 42, int furiganaFontSize = 20);
    ~PNGRenderer() = default;

    enum class TextJustify
    {
        Left,
        Center,
        Right,
    };

    enum class FuriganaDistance
    {
        None,
        Narrow,
        Far,
        Unchanged,
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

    inline void setFontSize(int fontSize)
    {
        _fontSize = fontSize;
    }

    inline void setFuriganaFontSize(int furiganaFontSize)
    {
        _furiganaFontSize = furiganaFontSize;
    }

    inline void setLineSpaceReduction(int lineSpaceReduction)
    {
        _lineSpaceReduction = lineSpaceReduction;
    }

    inline void setTextJustify(TextJustify textJustify)
    {
        _textJustify = textJustify;
    }

    inline void setFuriganaDistance(FuriganaDistance furiganaDistance)
    {
        _furiganaDistance = furiganaDistance;
    }

    const std::vector<char> render() const;

private:
    bool _vertical = false;
    std::string _text;
    std::string _fontFamily;
    int _fontSize = -1;
    int _furiganaFontSize = -1;
    int _lineSpaceReduction = 27;
    TextJustify _textJustify = TextJustify::Center;
    FuriganaDistance _furiganaDistance = FuriganaDistance::Narrow;
};

#endif // PNGRENDERER_HPP
