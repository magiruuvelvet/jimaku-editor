#include "styledsrtparser.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace SrtParser {

namespace {

// 19 normal properties
// 1 overwrite property
const style_hints_t default_hints = {

    // video dimensions (required for frame positioning in PGS)
    {"width",                       "1920"},
    {"height",                      "1080"},

    {"text-direction",              "horizontal"},
    {"text-alignment",              "center"},
    {"text-justify",                "center"},
    {"margin-bottom",               "150"},
    {"margin-side",                 "130"},
    {"margin-top",                  "90"},
    // avoid empty string as default, TODO: figure out what font Netflix Japan is using and make it the default
    {"font-family",                 "TakaoPGothic"},
    {"font-size",                   "46"},
    {"font-color",                  "#f1f1f1"},
    {"horizontal-numbers",          "true"},
    {"furigana-spacing",            "font"},
    {"furigana-distance",           "unchanged"},
    {"furigana-font-size",          "20"},
    {"furigana-font-color",         "#f1f1f1"},
    {"line-space-reduction",        "0"},
    {"furigana-line-space-reduction", "0"},
    {"border-color",                "#191919"},
    {"border-size",                 "3"},
    {"furigana-border-size",        "2"},
    {"blur-radius",                 "10"},
    {"blur-sigma",                  "0.5"},
    {"color-limit",                 "40"},

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

std::vector<StyledSubtitleItem> parse_helper(std::vector<SubtitleItem> &subs, bool *error)
{
    // global style hint is mandatory
    if (subs.empty())
    {
        if (error)
        {
            (*error) = true;
        }

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

} // anonymous namespace

unsigned StyledSubtitleItem::width() const
{
    try {
        return unsigned(std::stoul(property(Width)));
    } catch (...) {
        return 1920;
    }
}

unsigned StyledSubtitleItem::height() const
{
    try {
        return unsigned(std::stoul(property(Height)));
    } catch (...) {
        return 1080;
    }
}

unsigned long StyledSubtitleItem::fontSize() const
{
    try {
        return std::stoul(property(FontSize));
    } catch (...) {
        return 46;
    }
}

unsigned long StyledSubtitleItem::furiganaFontSize() const
{
    try {
        return std::stoul(property(FuriganaFontSize));
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

unsigned long StyledSubtitleItem::borderSize() const
{
    try {
        return std::stoul(property(BorderSize));
    } catch (...) {
        return 3;
    }
}

unsigned long StyledSubtitleItem::furiganaBorderSize() const
{
    try {
        return std::stoul(property(FuriganaBorderSize));
    } catch (...) {
        return 2;
    }
}

unsigned long StyledSubtitleItem::marginBottom() const
{
    try {
        return std::stoul(property(MarginBottom));
    } catch (...) {
        return 150;
    }
}

unsigned long StyledSubtitleItem::marginSide() const
{
    try {
        return std::stoul(property(MarginSide));
    } catch (...) {
        return 130;
    }
}

unsigned long StyledSubtitleItem::marginTop() const
{
    try {
        return std::stoul(property(MarginTop));
    } catch (...) {
        return 90;
    }
}

double StyledSubtitleItem::blurRadius() const
{
    try {
        return unsigned(std::stod(property(BlurRadius)));
    } catch (...) {
        return 10;
    }
}

double StyledSubtitleItem::blurSigma() const
{
    try {
        return unsigned(std::stod(property(BlurSigma)));
    } catch (...) {
        return 0.5;
    }
}

unsigned StyledSubtitleItem::colorLimit() const
{
    try {
        return unsigned(std::stoul(property(ColorLimit)));
    } catch (...) {
        return 40;
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

std::vector<StyledSubtitleItem> parseStyled(const std::string &fileName, bool *error, std::string *exception)
{
    auto subs = parse(fileName, error, exception);
    return parse_helper(subs, error);
}

std::vector<StyledSubtitleItem> parseStyledFromMemory(const std::string &contents, bool *error, std::string *exception)
{
    auto subs = parseFromMemory(contents, error, exception);
    return parse_helper(subs, error);
}

std::vector<StyledSubtitleItem> parseStyledWithExternalHints(const std::string &fileName, const std::string &hintData, bool *error, std::string *exception)
{
    // parse subtitles
    auto subs = parse(fileName, error, exception);

    if (error && (*error))
    {
        return {};
    }

    // parse hint data
    auto hints = parseFromMemory(hintData, error, exception);

    if (error && (*error))
    {
        return {};
    }

    // prepend to parsed subtitles
    subs.insert(subs.begin(), hints.begin(), hints.end());

    return parse_helper(subs, error);
}

} // namespace SrtParser
