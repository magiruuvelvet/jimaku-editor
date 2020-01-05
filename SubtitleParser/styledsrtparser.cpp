#include "styledsrtparser.hpp"

//#ifdef DEBUG_BUILD
//#include <iostream>
//#endif

#include <string>
#include <sstream>
#include <vector>

namespace SrtParser {

namespace {

// 19 normal properties
// 1 overwrite property
const style_hints_t default_hints = {
    {"text-direction",              "horizontal"},
    {"text-alignment",              "center"},
    {"text-justify",                "center"},
    {"margin-bottom",               "100"},
    {"margin-side",                 "100"},
    {"margin-top",                  "150"},
    // avoid empty string as default, TODO: figure out what font Netflix Japan is using and make it the default
    {"font-family",                 "TakaoPGothic"},
    {"font-size",                   "48"},
    {"font-color",                  "#f1f1f1"},
    {"horizontal-numbers",          "true"},
    {"furigana-spacing",            "font"},
    {"furigana-distance",           "unchanged"},
    {"furigana-font-size",          "20"},
    {"furigana-font-color",         "#f1f1f1"},
    {"line-space-reduction",        "0"},
    {"furigana-line-space-reduction", "0"},
    {"border-color",                "#191919"},
    {"border-size",                 "4"},
    {"furigana-border-size",        "2"},

    // overwrite properties: are setting one of the above during parsing
    // {"margin-overwrite"}
};

const style_hints_t validate_hints = {
    {"margin-overwrite",            ""},
};

static std::pair<std::string, std::string> parse_hint(const std::string &line)
{
    auto delim = line.find_first_of('=');

    if (delim == std::string::npos)
    {
        return {};
    }

    const auto property = line.substr(0, delim);
    const auto value = line.substr(delim + 1);

    // validate property
    if (default_hints.find(property) == default_hints.end() && validate_hints.find(property) == validate_hints.end())
    {
        // invalid property, return empty pair
        return {};
    }

    // return parsed and valid hint
    return {property, value};
}

static std::string extract_hints(const SubtitleItem &sub, style_hints_t &target_hints)
{
    std::stringstream frame{sub.text()};
    std::string line;

    std::string cleanLines;

    while (std::getline(frame, line))
    {
        if (line.rfind("# ", 0) == 0)
        {
            // parse hint in current line
            const auto hint = parse_hint(line.substr(2));

            // only insert non-empty hints
            if (hint.first.length() > 0 && hint.second.length() > 0)
            {
                auto res = target_hints.insert(hint);

                // make sure to overwrite existing properties with the new value
                if (!res.second)
                {
                    target_hints[hint.first] = hint.second;
                }
            }
        }
        else
        {
            cleanLines += line + '\n';
        }
    }

    // return subtitle text without style hint comments
    return cleanLines.substr(0, cleanLines.length() - 1);
}

} // anonymous namespace

int StyledSubtitleItem::fontSize() const
{
    try {
        return std::stoi(property(FontSize));
    } catch (...) {
        return 48;
    }
}

int StyledSubtitleItem::furiganaFontSize() const
{
    try {
        return std::stoi(property(FuriganaFontSize));
    } catch (...) {
        return 20;
    }
}

int StyledSubtitleItem::lineSpaceReduction() const
{
    try {
        return std::stoi(property(LineSpaceReduction));
    } catch (...) {
        return 0;
    }
}

int StyledSubtitleItem::furiganaLineSpaceReduction() const
{
    try {
        return std::stoi(property(FuriganaLineSpaceReduction));
    } catch (...) {
        return 0;
    }
}

int StyledSubtitleItem::borderSize() const
{
    try {
        return std::stoi(property(BorderSize));
    } catch (...) {
        return 4;
    }
}

int StyledSubtitleItem::furiganaBorderSize() const
{
    try {
        return std::stoi(property(FuriganaBorderSize));
    } catch (...) {
        return 2;
    }
}

int StyledSubtitleItem::marginBottom() const
{
    try {
        return std::stoi(property(MarginBottom));
    } catch (...) {
        return 100;
    }
}

int StyledSubtitleItem::marginSide() const
{
    try {
        return std::stoi(property(MarginSide));
    } catch (...) {
        return 100;
    }
}

int StyledSubtitleItem::marginTop() const
{
    try {
        return std::stoi(property(MarginTop));
    } catch (...) {
        return 150;
    }
}

bool StyledSubtitleItem::isVertical() const
{
    return property(TextDirection) == "vertical";
}

const std::string StyledSubtitleItem::get_property_value(const std::string &property) const
{
    // don't do anything on empty input
    if (property.empty())
    {
        return {};
    }

    if (_hints.find(property) != _hints.end())
    {
        return _hints.at(property);
    }

    return {};
}

std::vector<StyledSubtitleItem> parseStyled(const std::string &fileName)
{
    auto subs = parse(fileName);

    if (subs.empty())
    {
        return {};
    }

    // global style hints
    style_hints_t global_hints = default_hints;

    // extract global style hints (sub with number 0)
    extract_hints(subs.at(0), global_hints);

//#ifdef DEBUG_BUILD
//    for (auto&& global_hint : global_hints)
//    {
//        std::cerr << global_hint.first << " -> " << global_hint.second << std::endl;
//    }
//#endif

    // remove global style hint frame
    subs.erase(subs.begin());

    std::vector<StyledSubtitleItem> subtitles;

    // iterate over all subtitles
    for (auto&& unstyledSub : subs)
    {
        StyledSubtitleItem sub(unstyledSub);
        style_hints_t overwrite_hints = global_hints;
        sub.setText(extract_hints(unstyledSub, overwrite_hints));

        const auto &text_direction = overwrite_hints.at("text-direction");

        // check for overwrite properties
        if (overwrite_hints.find("margin-overwrite") != overwrite_hints.end())
        {
            const auto margin_overwrite = overwrite_hints.at("margin-overwrite");

            if (text_direction == "horizontal")
            {
                overwrite_hints["margin-bottom"] = margin_overwrite;
            }
            else if (text_direction == "vertical")
            {
                overwrite_hints["margin-side"] = margin_overwrite;
            }

            // remove overwrite properties
            overwrite_hints.erase(overwrite_hints.find("margin-overwrite"));
        }

        // set default text alignment to right on vertical when value is center
        if (text_direction == "vertical" && overwrite_hints["text-alignment"] == "center")
        {
            overwrite_hints["text-alignment"] = "right";
        }

        sub.setStyleHints(overwrite_hints);
        subtitles.emplace_back(sub);
    }

    return subtitles;
}

} // namespace SrtParser
