#ifndef STYLEDSRTPARSER_HPP
#define STYLEDSRTPARSER_HPP

#include "srtparser.hpp"

#include <map>

namespace SrtParser {

// data types
using style_hints_t = std::map<std::string, std::string>;

class StyledSubtitleItem : public SubtitleItem
{
public:
    enum Properties
    {
        Width,
        Height,
        TextDirection,
        TextAlignment,
        TextJustify,
        MarginBottom,
        MarginSide,
        MarginTop,
        MarginOverwrite,
        FontFamily,
        FontSize,
        FontColor,
        HorizontalNumbers,
        FuriganaSpacing,
        FuriganaDistance,
        FuriganaFontSize,
        FuriganaFontColor,
        LineSpaceReduction,
        FuriganaLineSpaceReduction,
        BorderColor,
        BorderSize,
        FuriganaBorderSize,
        BlurRadius,
        BlurSigma,
        ColorLimit,
    };

    StyledSubtitleItem()
    {}

    StyledSubtitleItem(const SubtitleItem &unstyled)
    {
        this->setText(unstyled.text());
        this->setSubNumber(unstyled.subNumber());
        this->setStartTime(unstyled.startTime());
        this->setEndTime(unstyled.endTime());
    }

    StyledSubtitleItem(sub_number_t subNo, const std::string &startTime, const std::string &endTime, const std::string &text)
    {
        this->setText(text);
        this->setSubNumber(subNo);
        this->setStartTime(this->timeMSec(startTime));
        this->setEndTime(this->timeMSec(endTime));
    }

    ~StyledSubtitleItem() = default;

    // set style hints
    inline void setStyleHints(const style_hints_t &hints)
    {
        _hints = hints;
    }

    // get style hints
    inline const style_hints_t &styleHints() const
    {
        return _hints;
    }

    // get property value
    // if value is not present in item, return global default
    // if the property is not part of the spec, an empty string is returned
    inline const std::string property(const std::string &property) const
    {
        return this->get_property_value(property);
    }

    inline const std::string property(Properties property) const
    {
        return this->get_property_value(property_string(property));
    }

    // video dimensions
    unsigned width() const;
    unsigned height() const;

    unsigned long fontSize() const;
    unsigned long furiganaFontSize() const;
    int lineSpaceReduction() const;
    int furiganaLineSpaceReduction() const;

    unsigned long borderSize() const;
    unsigned long furiganaBorderSize() const;

    unsigned long marginBottom() const;
    unsigned long marginSide() const;
    unsigned long marginTop() const;

    double blurRadius() const;
    double blurSigma() const;

    unsigned colorLimit() const;

    bool isVertical() const;

protected:
    const std::string get_property_value(const std::string &property) const;

    // convert properties enum to property string
    static const std::string property_string(Properties property)
    {
        switch (property)
        {
            case Width:                 return "width";
            case Height:                return "height";
            case TextDirection:         return "text-direction";
            case TextAlignment:         return "text-alignment";
            case TextJustify:           return "text-justify";
            case MarginBottom:          return "margin-bottom";
            case MarginSide:            return "margin-side";
            case MarginTop:             return "margin-top";
            case MarginOverwrite:       return "margin-overwrite";
            case FontFamily:            return "font-family";
            case FontSize:              return "font-size";
            case FontColor:             return "font-color";
            case HorizontalNumbers:     return "horizontal-numbers";
            case FuriganaSpacing:       return "furigana-spacing";
            case FuriganaDistance:      return "furigana-distance";
            case FuriganaFontSize:      return "furigana-font-size";
            case FuriganaFontColor:     return "furigana-font-color";
            case LineSpaceReduction:    return "line-space-reduction";
            case FuriganaLineSpaceReduction: return "furigana-line-space-reduction";
            case BorderColor:           return "border-color";
            case BorderSize:            return "border-size";
            case FuriganaBorderSize:    return "furigana-border-size";
            case BlurRadius:            return "blur-radius";
            case BlurSigma:             return "blur-sigma";
            case ColorLimit:            return "color-limit";
        }
    }

private:
    style_hints_t _hints;
};

std::vector<StyledSubtitleItem> parseStyled(const std::string &fileName, bool *error = nullptr, std::string *exception = nullptr);

} // namespace SrtParser

#endif // STYLEDSRTPARSER_HPP
