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
        TextDirection,
        TextAlignment,
        TextJustify,
        MarginBottom,
        MarginSide,
        MarginTop,
        MarginOverwrite,
        FontFamily,
        FontSize,
        HorizontalNumbers,
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
    inline const std::string property(const std::string &property)
    {
        return this->get_property_value(property);
    }

    inline const std::string property(Properties property)
    {
        return this->get_property_value(property_string(property));
    }

protected:
    const std::string get_property_value(const std::string &property);

    // convert properties enum to property string
    static const std::string property_string(Properties property)
    {
        switch (property)
        {
            case TextDirection:     return "text-direction";
            case TextAlignment:     return "text-alignment";
            case TextJustify:       return "text-justify";
            case MarginBottom:      return "margin-bottom";
            case MarginSide:        return "margin-side";
            case MarginTop:         return "margin-top";
            case MarginOverwrite:   return "margin-overwrite";
            case FontFamily:        return "font-family";
            case FontSize:          return "font-size";
            case HorizontalNumbers: return "horizontal-numbers";
            default: return {};
        }
    }

private:
    style_hints_t _hints;
};

std::vector<StyledSubtitleItem> parseStyled(const std::string &fileName);

} // namespace SrtParser

#endif // STYLEDSRTPARSER_HPP
