#include <iostream>

#include <srtparser.hpp>

namespace srtparser_tests {

bool parse_basic()
{
    const auto srt_file = std::string{UNIT_TEST_CURRENT_DIR} + "/test.ja.srt";

    auto subs = SrtParser::parse(srt_file);

    // validate lines
    auto linesCheck =
        subs.at(0).text() == "（鳥のさえずり）" &&
        subs.at(1).text() == "（宮内(みやうち)れんげ）おおーっ！" &&
        subs.at(4).text() == "私の時は 姉ちゃんの\nお下がりだったのにな" && // new line test and carriage return removal test
        subs.at(252).text() == "…で こっちは 縦笛";

    // validate sub number
    auto subNoCheck =
        subs.at(0).subNumber() == 1 &&
        subs.at(1).subNumber() == 2 &&
        subs.at(4).subNumber() == 5 &&
        subs.at(253).subNumber() == 254;

    return linesCheck && subNoCheck;
}

} // namespace srtparser_tests
