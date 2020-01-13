#include <iostream>
#include <filesystem>

#include <config/version.hpp>

#include "libs/cxxopts.hpp"

#include <srtparser/styledsrtparser.hpp>
#include <renderer/pgsframecreator.hpp>

int main(int argc, char **argv)
{
    // copy argc, cxxopts modifies this and argv
    int orig_argc = argc;

    std::cout << "jimaku-renderer " << version::get() << std::endl;

    cxxopts::Options options("jimaku-renderer", "Subtitle renderer for CJK subtitles");

    // required options
    options.add_options("required options")
      ("f,srt-file",   "Input styled SRT file", cxxopts::value<std::string>())
      ("o,output-dir", "Target directory to write rendered subtitles too", cxxopts::value<std::string>())
      ;

    // optimal options
    options.add_options("optimal options")
      ("command",      "Run a command on all PNG files (file placeholder is %f)", cxxopts::value<std::string>())
      ("hints",        "Use external styling hints (overwrites global hints in srt file)", cxxopts::value<std::string>())
      ;

    // debug options
    options.add_options("debug options")
      ("v,verbose",    "Enable verbose output")
      ;

    // misc options
    options.add_options("help options")
      ("h,help",       "Show this help")
      ("version",      "Show application version")
      ;

    // hack to not put the entire program logic into a try catch block
    // works around a library restriction
    std::vector<cxxopts::ParseResult> result_scope_helper;
    try {
        // parse and validate arguments, throws an exception on errors
        result_scope_helper.emplace_back(options.parse(argc, argv));
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    const auto &result = result_scope_helper.at(0);

    // show version
    if (result["version"].as<bool>())
    {
        std::cout << version::get() << std::endl;
        return 0;
    }

    // show help text
    if (result["help"].as<bool>() || orig_argc == 1)
    {
        std::cout << options.help() << std::endl;
        return 0;
    }

    bool isVerbose = result.count("verbose") == 1 && result["verbose"].as<bool>();
    bool hasSrt = result.count("srt-file") == 1;
    bool hasOutDir = result.count("output-dir") == 1;
    bool hasCommand = result.count("command") == 1;
    bool hasExternalHints = result.count("hints") == 1;

    if (!hasSrt)
    {
        std::cerr << "error: exactly one srt file must be provided" << std::endl;
        return 1;
    }

    if (!hasOutDir)
    {
        std::cerr << "error: exactly one output directory must be provided" << std::endl;
        return 1;
    }

    // parse srt file
    const auto srt_file = result["srt-file"].as<std::string>();
    std::cout << "parsing srt file: " << srt_file << std::endl;

    bool error;
    std::string exception;
    std::vector<SrtParser::StyledSubtitleItem> subtitles;

    if (hasExternalHints)
    {
        const auto fileName = result["hints"].as<std::string>();

        std::cout << "using external hints file: " << fileName << std::endl;

        if (!(std::filesystem::exists(fileName) && std::filesystem::is_regular_file(fileName)))
        {
            std::cerr << "error: hints file does not exist" << std::endl;
            return 1;
        }

        const auto hintData = SrtParser::readFile(fileName);
        subtitles = SrtParser::parseStyledWithExternalHints(srt_file, hintData, &error, &exception);
    }
    else
    {
        subtitles = SrtParser::parseStyled(srt_file, &error, &exception);
    }

    if (error)
    {
        std::cerr << "error: parsing of srt file failed" << std::endl;
        if (!exception.empty())
        {
            std::cerr << "reason: " << exception << std::endl;
        }
        return 1;
    }

    if (subtitles.empty())
    {
        std::cout << "error: subtitle has no frames" << std::endl;
        return 1;
    }

    const std::string command = hasCommand ? result["command"].as<std::string>() : "";

    // create batch renderer
    std::cout << "initializing renderer..." << std::endl;
    PGSFrameCreator pgs(subtitles, subtitles.at(0).width(), subtitles.at(1).height());
    pgs.setCommand(command);

    const auto out_path = result["output-dir"].as<std::string>();
    std::cout << "frames will be written to: " << out_path << std::endl;

    bool command_is_dangerous = false;
    const auto final_command = pgs.commandTemplate(&command_is_dangerous);
    if (!final_command.empty())
    {
        if (command_is_dangerous)
        {
            std::cout << "command is considered dangerous and was disabled" << std::endl;
            std::cout << "command is: " << final_command << std::endl;
        }
        else
        {
            std::cout << "command to run on all frames: " << final_command << std::endl;
        }
    }

    // start rendering
    auto status = pgs.render(out_path, isVerbose);

    if (status == PGSFrameCreator::DirectoyNotCreated)
    {
        std::cerr << "error: unable to create target directory" << std::endl;
        return 1;
    }
    else if (status == PGSFrameCreator::FileNotCreated)
    {
        std::cerr << "error: unable to create files inside the target directory" << std::endl;
        return 1;
    }

    return 0;
}
