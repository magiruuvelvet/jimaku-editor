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
    PNGRenderer(const std::string &text, const std::string &fontFamily = {}, int fontSize = -1);
    ~PNGRenderer() = default;

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

    const std::vector<char> render() const;

private:
    std::string _text;
    std::string _fontFamily;
    int _fontSize = -1;
};

#endif // PNGRENDERER_HPP
