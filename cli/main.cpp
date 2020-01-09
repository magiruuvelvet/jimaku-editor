#include <iostream>

#include <config/version.hpp>

#include "cxxopts.hpp"

#include <srtparser/styledsrtparser.hpp>
#include <renderer/pgsframecreator.hpp>

int main(int argc, char **argv)
{
    // copy argc, cxxopts modifies this and argv
    int orig_argc = argc;

    cxxopts::Options options("jimaku-renderer", "Subtitle renderer for CJK subtitles");

    options.add_options()
      ("h,help",       "Show this help")
      ("version",      "Show application version")
      ("v,verbose",    "Enable verbose output")
      ("f,srt-file",   "Input styled SRT file", cxxopts::value<std::string>())
      ("o,output-dir", "Target directory to write rendered subtitles too", cxxopts::value<std::string>())
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
    const auto subtitles = SrtParser::parseStyled(srt_file, &error, &exception);

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

    // create batch renderer
    std::cout << "initializing renderer..." << std::endl;
    PGSFrameCreator pgs(subtitles, subtitles.at(0).width(), subtitles.at(1).height());

    const auto out_path = result["output-dir"].as<std::string>();
    std::cout << "frames will be written to: " << out_path << std::endl;

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

    std::cout << "all frames rendered" << std::endl;

    return 0;
}
