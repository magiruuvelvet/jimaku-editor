#include <iostream>

#include <srtparser/srtparser.hpp>
#include <srtparser/styledsrtparser.hpp>

namespace srtparser_tests {

bool parse_basic()
{
    const auto srt_file = std::string{UNIT_TEST_CURRENT_DIR} + "/test.ja.srt";

    const auto subs = SrtParser::parse(srt_file);

    // validate length
    auto lengthCheck = subs.size() == 270;

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

    // validate timestamps
    auto timestampCheck =
        subs.at(0).startTime() == 1626 && subs.at(0).endTime() == 8383 &&
        subs.at(1).startTime() == 16599 && subs.at(1).endTime() == 18935 &&
        subs.at(4).startTime() == 24691 && subs.at(4).endTime() == 28403 &&
        subs.at(253).startTime() == 1262511 && subs.at(253).endTime() == 1264263;

    return lengthCheck && linesCheck && subNoCheck && timestampCheck;
}

bool parse_styled()
{
    const auto srt_file = std::string{UNIT_TEST_CURRENT_DIR} + "/test_custom.ja.srt";

    const auto subs = SrtParser::parseStyled(srt_file);

    // validate length (first frame [0] is removed after parsing)
    auto lengthCheck = subs.size() == 270;

    // validate lines
    auto linesCheck =
        subs.at(0).text() == "（{鳥|とり}のさえずり）" &&
        subs.at(1).text() == "（{宮内|みやうち}れんげ）おおーっ！" &&
        subs.at(4).text() == "私の時は 姉ちゃんの\nお下がりだったのにな" && // new line test and carriage return removal test
        subs.at(252).text() == "…で こっちは 縦笛";

    // validate sub number
    auto subNoCheck =
        subs.at(0).subNumber() == 1 &&
        subs.at(1).subNumber() == 2 &&
        subs.at(4).subNumber() == 5 &&
        subs.at(253).subNumber() == 254;

    // validate timestamps
    auto timestampCheck =
        subs.at(0).startTime() == 1626 && subs.at(0).endTime() == 8383 &&
        subs.at(1).startTime() == 16599 && subs.at(1).endTime() == 18935 &&
        subs.at(4).startTime() == 24691 && subs.at(4).endTime() == 28403 &&
        subs.at(253).startTime() == 1262511 && subs.at(253).endTime() == 1264263;

    return lengthCheck && linesCheck && subNoCheck && timestampCheck;
}

bool parse_styled_external_hints()
{
    const auto srt_file = std::string{UNIT_TEST_CURRENT_DIR} + "/test_custom.ja.srt";

    const std::string hints =
"0\n"
"00:00:00,000 --> 00:00:00,000\n"
"# text-direction=horizontal\n"
"# line-space-reduction=2\n"
"# furigana-line-space-reduction=2\n"
"\n";

    const auto subs = SrtParser::parseStyledWithExternalHints(srt_file, hints);

    return
        subs.size() == 270 &&
        subs.at(0).furiganaLineSpaceReduction() == 2 &&
        subs.at(0).property(SrtParser::StyledSubtitleItem::TextDirection) == "horizontal";
}

} // namespace srtparser_tests
