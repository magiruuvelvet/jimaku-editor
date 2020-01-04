#include "styledsrtparser.hpp"

//#ifdef DEBUG_BUILD
//#include <iostream>
//#endif

#include <string>
#include <sstream>
#include <vector>

namespace SrtParser {

namespace {

const style_hints_t default_hints = {
    {"text-direction",          "horizontal"},
    {"text-alignment",          "center"},
    {"text-justify",            "center"},
    {"margin-bottom",           "100"},
    {"margin-side",             "100"},
    {"margin-top",              "150"},
    {"font-family",             "Noto Sans CJK JP"}, // avoid empty string as default, TODO: figure out what font Netflix Japan is using and make it the default
    {"font-size",               "42"},
    {"font-color",              "#f1f1f1"},
    {"horizontal-numbers",      "true"},
    {"furigana-spacing",        "font"},
    {"furigana-distance",       "narrow"},
    {"furigana-font-size",      "20"},
    {"furigana-font-color",     "#f1f1f1"},
    {"line-space-reduction",    "27"},
};

static std::pair<std::string, std::string> parse_hint(const std::string &line)
{
    auto delim = line.find_first_of('=');

    if (delim == std::string::npos)
    {
        return {};
    }

    return {
        line.substr(0, delim),
        line.substr(delim + 1)
    };
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

const std::string StyledSubtitleItem::get_property_value(const std::string &property)
{
    // don't do anything on empty input
    if (property.empty())
    {
        return {};
    }
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
        sub.setStyleHints(overwrite_hints);
        subtitles.emplace_back(sub);
    }

    return subtitles;
}

} // namespace SrtParser
