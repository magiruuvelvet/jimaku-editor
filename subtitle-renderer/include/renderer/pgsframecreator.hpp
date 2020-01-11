#ifndef PGSFRAMECREATOR_HPP
#define PGSFRAMECREATOR_HPP

#include <string>
#include <vector>

#include <srtparser/styledsrtparser.hpp>

class PGSFrameCreator
{
public:
    PGSFrameCreator();
    PGSFrameCreator(const std::vector<SrtParser::StyledSubtitleItem> &subtitles, unsigned videoWidth = 1920, unsigned videoHeight = 1080);
    ~PGSFrameCreator() = default;

    void setCommand(const std::string &command);

    const std::string commandTemplate(bool *is_dangerous = nullptr) const;

    enum ErrorCode
    {
        Success = 0,
        DirectoyNotCreated,
        FileNotCreated,
    };

    ErrorCode render(const std::string &out_path, bool verbose = false) const;

private:
    std::vector<SrtParser::StyledSubtitleItem> _subtitles;
    unsigned _width = 1920;
    unsigned _height = 1080;

    std::string _command;
    std::vector<std::string> _args_template;
    bool _command_contains_dangerous = false;
};

#endif // PGSFRAMECREATOR_HPP
