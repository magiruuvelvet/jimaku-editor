#include "srtparser.hpp"

#include <cstdlib>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace SrtParser {

namespace {

static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim))
    {
        elems.emplace_back(item);
    }

    return elems;
}

} // anonymous namespace

std::vector<SubtitleItem> parse(const std::string &fileName)
{
    std::vector<SubtitleItem> subtitles;

    std::ifstream infile(fileName);
    std::string line, start, end, completeLine = "", timeLine = "";
    sub_number_t subNo = 0;
    auto turn = 0;

    /*
     * turn = 0 -> Add subtitle number
     * turn = 1 -> Add string to timeLine
     * turn > 1 -> Add string to completeLine
     */

    while (std::getline(infile, line))
    {
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());

        if (line.compare(""))
        {
            if (!turn)
            {
                try {
                    subNo = std::stoull(line);
                } catch (...) {
                    subNo = 0;
                }

                turn++;
                continue;
            }

            if (line.find("-->") != std::string::npos)
            {
                timeLine += line;

                std::vector<std::string> srtTime;
                srtTime = split(timeLine, ' ', srtTime);
                start = srtTime[0];
                end = srtTime[2];
            }
            else
            {
                if (completeLine != "")
                {
                    // preserve line breaks
                    completeLine += "\n";
                }

                completeLine += line;
            }

            turn++;
        }
        else
        {
            turn = 0;
            subtitles.emplace_back(SubtitleItem(subNo, start, end, completeLine));
            completeLine = timeLine = "";
        }

        if (infile.eof())
        {
            subtitles.emplace_back(SubtitleItem(subNo, start, end, completeLine));
        }
    }

    return subtitles;
}

timestamp_t SubtitleItem::timeMSec(const std::string &value)
{
    std::vector<std::string> t, secs;
    timestamp_t hours, mins, seconds, milliseconds;

    t = split(value, ':', t);
    hours = std::stoull(t[0]);
    mins = std::stoull(t[1]);

    secs = split(t[2], ',', secs);
    seconds = std::stoull(secs[0]);
    milliseconds = std::stoull(secs[1]);

    return hours * 3600000 + mins * 60000 + seconds * 1000 + milliseconds;
}

} // namespace SrtParser
