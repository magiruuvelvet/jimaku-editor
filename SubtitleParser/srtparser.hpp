/**
 * Basic SRT file parser
 *
 * Parses all subtitles of a SRT file into SubtitleItem objects.
 *
 * A SubtitleItem contains:
 *  -> raw sub text (locale agnostic, no additional parsing is done here)
 *  -> subtitle number
 *  -> start and end time as string
 *  -> start and end time parsed and converted to milliseconds
 *
 */

#ifndef SRTPARSER_HPP
#define SRTPARSER_HPP

#include <string>
#include <vector>
#include <memory>

namespace SrtParser {

// data types
using timestamp_t = unsigned long long;
using sub_number_t = unsigned long;

class SubtitleItem
{
public:
    SubtitleItem()
    {}

    SubtitleItem(sub_number_t subNo, const std::string &startTime, const std::string &endTime, const std::string &text)
        : _text(text)
        , _subNo(subNo)
        , _startTimeString(startTime)
        , _endTimeString(endTime)
    {
        _startTime = this->timeMSec(startTime);
        _endTime = this->timeMSec(endTime);
    }

    ~SubtitleItem() = default;

    // start time in ms
    inline timestamp_t startTime() const
    {
        return _startTime;
    }

    // end time in ms
    inline timestamp_t endTime() const
    {
        return _endTime;
    }

    // subtitle text as present in the srt file with preserved line breaks
    inline const std::string &text() const
    {
        return _text;
    }

    // subtitle number
    inline sub_number_t subNumber() const
    {
        return _subNo;
    }

    // start time string as present in the srt file
    inline const std::string &startTimeString() const
    {
        return _startTimeString;
    }

    // end time string as present in the srt file
    inline const std::string &endTimeString() const
    {
        return _endTimeString;
    }

    // set start time in ms
    inline void setStartTime(timestamp_t startTime)
    {
        _startTime = startTime;
    }

    // set end time in ms
    inline void setEndTime(timestamp_t endTime)
    {
        _endTime = endTime;
    }

    // set subtitle text
    inline void setText(const std::string &text)
    {
        _text = text;
    }

    // set subtitle number
    inline void setSubNumber(sub_number_t subNo)
    {
        _subNo = subNo;
    }

protected:
    timestamp_t timeMSec(const std::string &value);

private:
    timestamp_t _startTime;
    timestamp_t _endTime;
    std::string _text;

    sub_number_t _subNo;
    std::string _startTimeString;
    std::string _endTimeString;
};

std::vector<SubtitleItem> parse(const std::string &fileName, bool *error = nullptr, std::string *exception = nullptr);

} // namespace SrtParser

#endif //SRTPARSER_HPP
