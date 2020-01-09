#include "srtparser.hpp"

#include <cstdlib>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>

#include <filesystem>

namespace SrtParser {

namespace {

// split a given string at delimiter and push results into elems
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

// silently ignore the BOM when present, assume file is in UTF-8 by default
static void skip_utf8_bom(std::ifstream &fs)
{
    int dst[3];

    for (auto& i : dst)
    {
        i = fs.get();
    }

    constexpr int utf8[] = { 0xEF, 0xBB, 0xBF };

    if (!std::equal(std::begin(dst), std::end(dst), utf8))
    {
        fs.seekg(0);
    }
}

} // anonymous namespace

std::vector<SubtitleItem> parse(const std::string &fileName, bool *error, std::string *exception)
{
    std::vector<SubtitleItem> subtitles;

    // check if file exists before attempting to read it
    if (!(std::filesystem::exists(fileName) && std::filesystem::is_regular_file(fileName)))
    {
        if (error)
        {
            (*error) = true;
        }

        return {};
    }

    // read file and ignore BOM when present (causing parsing issues with std::getline)
    std::ifstream infile(fileName, std::ios::in);
    skip_utf8_bom(infile);

    std::string line, start, end, completeLine = "", timeLine = "";
    sub_number_t subNo = 0;
    auto turn = 0;

    /*
     * turn = 0 -> Add subtitle number
     * turn = 1 -> Add string to timeLine
     * turn > 1 -> Add string to completeLine
     */

    try
    {
        while (std::getline(infile, line))
        {
            // remove carriage return line breaks from line (only keep line feeds)
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

            if (!line.empty())
            {
                if (!turn)
                {
                    subNo = std::stoull(line);

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

    }
    catch (std::exception &e)
    {
        if (error)
        {
            (*error) = true;
        }

        if (exception)
        {
            (*exception) = e.what();
        }

        return {};
    }

    if (error)
    {
        (*error) = false;
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
