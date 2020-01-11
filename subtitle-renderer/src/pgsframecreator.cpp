#include "pgsframecreator.hpp"
#include "pngrenderer.hpp"

#include <reproc++/reproc.hpp>
#include <reproc++/sink.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>

using SrtParser::StyledSubtitleItem;

namespace {

static void write(const std::string &filename, const std::vector<char> &data)
{
    // write data to disk
    std::ofstream file(filename, std::ios::binary);
    file.write(data.data(), unsigned(data.size()));
    file.close();
}

static std::string format_duration(SrtParser::timestamp_t _ms)
{
    std::chrono::milliseconds ms(_ms);

    using namespace std::chrono;
    auto secs = duration_cast<seconds>(ms);
    ms -= duration_cast<milliseconds>(secs);
    auto mins = duration_cast<minutes>(secs);
    secs -= duration_cast<seconds>(mins);
    auto hour = duration_cast<hours>(mins);
    mins -= duration_cast<minutes>(hour);

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hour.count() << ":";
    ss << std::setfill('0') << std::setw(2) << mins.count() << ":";
    ss << std::setfill('0') << std::setw(2) << secs.count() << ".";
    ss << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// trim from start (in place)
static void ltrim(std::string &str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static void rtrim(std::string &str)
{
    str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

// trim from both ends (in place)
static void trim(std::string &str)
{
    ltrim(str);
    rtrim(str);
}

} // anonymous namespace

PGSFrameCreator::PGSFrameCreator()
{
}

PGSFrameCreator::PGSFrameCreator(const std::vector<SrtParser::StyledSubtitleItem> &subtitles, unsigned videoWidth, unsigned videoHeight)
    : _subtitles(subtitles),
      _width(videoWidth),
      _height(videoHeight)
{
}

PGSFrameCreator::ErrorCode PGSFrameCreator::render(const std::string &_out_path, bool verbose) const
{
    // check if out path exists and create it
    auto out_path = QString::fromUtf8(_out_path.c_str());
    auto relative_path = QFileInfo(out_path).isRelative();
    if (relative_path)
    {
        out_path.prepend(QDir::currentPath() + "/");
    }
    QDir out(QString::fromUtf8(_out_path.c_str()));
    if (!out.exists())
    {
        if (!QDir::root().mkpath(out_path))
        {
            return DirectoyNotCreated;
        }
    }

    const std::string full_out_path = out_path.toUtf8().constData();

    // create and open definition_file file
    QFile definition_file(out_path + "/pgs.xml");
    if (!definition_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return FileNotCreated;
    }

    QTextStream stream(&definition_file);

    // write command to run as xml comment
    const std::string pgssup_command = "pgssup -s " + std::to_string(_width) + "x" + std::to_string(_height) + " pgs.xml out.sup";
    stream << "<!-- command: " << pgssup_command.c_str() << " -->\n";

    // open xml segment
    stream << "<pgssup defaultoffset=\"0,0\">\n";
    stream.flush();

    // start processing all subtitles
    unsigned frameNo = 1;
    for (auto&& sub : _subtitles)
    {
        std::cout << "Rendering frame " << frameNo << "/" << _subtitles.size() << "... ";

        // setup renderer
        PNGRenderer renderer(sub.text(), sub.property(StyledSubtitleItem::FontFamily),
                             sub.fontSize(), sub.furiganaFontSize());
        renderer.setColorLimit(sub.colorLimit());

        // text direction and justification
        renderer.setVertical(sub.isVertical());
        renderer.setTextJustify(sub.property(StyledSubtitleItem::TextJustify));
        renderer.setLineSpaceReduction(sub.lineSpaceReduction());
        renderer.setFuriganaLineSpaceReduction(sub.furiganaLineSpaceReduction());
        renderer.setFuriganaDistance(sub.property(StyledSubtitleItem::FuriganaDistance));

        // font
        renderer.setFontColor(sub.property(StyledSubtitleItem::FontColor));
        renderer.setFuriganaFontColor(sub.property(StyledSubtitleItem::FuriganaFontColor));

        // border
        renderer.setBorderColor(sub.property(StyledSubtitleItem::BorderColor));
        renderer.setBorderSize(sub.borderSize());
        renderer.setFuriganaBorderSize(sub.furiganaBorderSize());
        renderer.setBlurRadius(sub.blurRadius());
        renderer.setBlurSigma(sub.blurSigma());

        if (verbose)
        {
            std::endl(std::cout);

            for (auto&& hint : sub.styleHints())
            {
                std::cout << " " << hint.first << " = " << hint.second << std::endl;
            }
        }

        // render subtitle image
        PNGRenderer::size_t size;
        PNGRenderer::pos_t pos;
        unsigned long color_count;
        const auto sub_image = renderer.render(&size, &pos, &color_count);

        // FIXME: not calculated correctly
        const auto size_as_8bit_pal = size.width * size.height;

        // H: left, center (default), right    V: right (default), left
        const auto alignment = sub.property(StyledSubtitleItem::TextAlignment);
        const bool vertical = sub.isVertical();
        const auto marginBottom = sub.marginBottom();
        const auto marginTop = sub.marginTop();
        const auto marginSide = sub.marginSide();

        // final coordinates of the subtitle image
        unsigned long x = 0, y = 0;

        // vertical placement
        if (vertical)
        {
            if (alignment == "right")
            {
                // align right main line directly at margin line and Furigana on right on the right side of the margin
                if (verbose)
                {
                    std::cout << " x = ";
                    std::cout << _width << "-" << size.width << "-" << marginSide << "+" << "(" << size.width << "-" << pos.x << ")" << std::endl;
                }

                // last calculation must happen in signed to prevent underflow
                x = _width - size.width - marginSide + ((long long) size.width - (long long) pos.x);
            }
            else if (alignment == "left")
            {
                x = marginSide;
            }
            else
            {
                std::cout << "warning: unknown alignment: " << alignment << std::endl;
            }

            y = marginTop;
        }

        // horizontal placement
        else
        {
            if (alignment == "left")
            {
                x = marginSide;
            }
            else if (alignment == "center")
            {
                x = (_width / 2) - (size.width / 2);
            }
            else if (alignment == "right")
            {
                x = _width - marginSide - size.width;
            }
            else
            {
                std::cout << "warning: unknown alignment: " << alignment << std::endl;
            }

            // align last main line above margin line and Furigana on bottom below margin
            if (verbose)
            {
                std::cout << " y = ";
                std::cout << _height << "-" << size.height << "-" << marginBottom << "+" << "(" << size.height << "-" << pos.y << ")" << std::endl;
            }

            // last calculation must happen in signed to prevent underflow
            y = _height - size.height - marginBottom + ((long long) size.height - (long long) pos.y);
        }

        if (verbose)
        {
            std::cout << " rendered image size = " << size.width << "x" << size.height << " (" << size_as_8bit_pal << " bytes in PGS)" << std::endl;
            std::cout << " calculated position offset = " << pos.x << "x" << pos.y << std::endl;
            std::cout << " calculated image position = " << x << "x" << y << std::endl;
        }

        // write sub image to disk
        const auto filename = std::to_string(frameNo) + ".png";
        const auto full_file_path = full_out_path + "/" + filename;
        write(full_file_path, sub_image);

        // write color count report with optimal warning
        if (color_count <= 255)
        {
            if (verbose)
            {
                std::cout << " unique colors = " << color_count << std::endl;
            }
            else
            {
                std::cout << "done [" << color_count << " unique colors in image]" << std::endl;
            }
        }
        else
        {
            if (verbose)
            {
                std::cout << " unique colors = " << color_count << " (warning: exceeded limit of 255 allowed colors)" << std::endl;
            }
            else
            {
                std::cout << "done [warning: " << color_count << " unique colors in image of 255 max allowed]" << std::endl;
            }
        }

        // print a warning when image size exceeds the maximum length a PGS frame can store
        if (size_as_8bit_pal > 0xFFFF)
        {
            std::cout << "warning: frame " << frameNo << " exceeds the maximum allowed " << 0xFFFF << " bytes by " << size_as_8bit_pal - 0xFFFF << " bytes" << std::endl;
        }

        // format time and write subtitle frame information to definition file
        auto start = format_duration(sub.startTime());
        auto end = format_duration(sub.endTime());

        stream << "    " <<
            "<subtitle " <<
                "starttime=\"" << start.c_str() << "\" " <<
                "endtime=\"" << end.c_str() << "\" " <<
                "offset=\"" << x << ',' << y << "\" " <<
                "image=\"" << frameNo << ".png\" />\n";
        stream.flush();

        // increment frame number
        ++frameNo;

        // execute optimal command on the PNG file
        if (!_command.empty())
        {
            // check if arguments are present in the template
            if (_args_template.empty())
            {
                std::cout << "warning: command is empty" << std::endl;
                continue;
            }

            // copy argument template
            auto args = _args_template;

            // replace %f with full png file path
            bool got_file_placeholder = false;
            for (auto i = 0U; i < args.size(); ++i)
            {
                if (args.at(i) == "%f")
                {
                    args[i] = full_file_path;
                    got_file_placeholder = true;
                    break;
                }
            }

            // check if file placeholder was present
            if (!got_file_placeholder)
            {
                std::cout << "warning: command is missing the png file path" << std::endl;
                continue;
            }

            // create process and options
            reproc::process process;
            reproc::options options;
            options.stop = {
                { reproc::stop::noop, reproc::milliseconds(0) },
                { reproc::stop::terminate, reproc::milliseconds(5000) },
                { reproc::stop::kill, reproc::milliseconds(2000) },
            };

            // execute user command
            std::error_code ec = process.start(args, options);

            // check for operating system errors
            if (ec)
            {
                std::cerr << "os error: " << ec.value() << ": " << ec.message() << std::endl;
                continue; // nothing more to do here, continue to next frame
            }

            // fetch application output
            std::string output, error;
            reproc::sink::string sink_out(output);
            reproc::sink::string sink_err(error);
            ec = reproc::drain(process, sink_out, sink_err);

            // check for operating system errors
            if (ec)
            {
                std::cerr << "os error: " << ec.value() << ": " << ec.message() << std::endl;
                continue; // nothing more to do here, continue to next frame
            }

            // maximum wait time before killing the process after graceful exit request
            options.stop.first = { reproc::stop::wait, reproc::milliseconds(10000) };

            // receive status code
            int status = 0;
            std::tie(status, ec) = process.stop(options.stop);

            // check for operating system errors
            if (ec)
            {
                std::cerr << "os error: " << ec.value() << ": " << ec.message() << std::endl;
                continue; // nothing more to do here, continue to next frame
            }

            if (status != 0)
            {
                std::cout << "warning: process did not end normally. got exit status: " << status << std::endl;

                if (verbose)
                {
                    std::cerr << "output stream:\n" << output << std::endl;
                    std::cerr << "error stream:\n" << error << std::endl;
                }
            }
        }
    }

    // close xml segment
    stream << "</pgssup>\n";
    stream.flush();

    std::cout << "all frames rendered, now you can run: " << pgssup_command << std::endl;

    // close definition file
    definition_file.close();
    return Success;
}

void PGSFrameCreator::setCommand(const std::string &command)
{
    _command = command;
    _command_contains_dangerous = false;

    // skip trimming when already empty
    if (_command.empty())
    {
        return;
    }

    trim(_command);

    // create arguments template for reproc (just split at whitespace, this is not a shell interpreter)
    _args_template.clear();

    if (_command.empty())
    {
        return;
    }

    std::istringstream splitter(_command);
    std::string arg;
    while (std::getline(splitter, arg, ' '))
    {
        trim(arg);
        if (!arg.empty())
        {
            _args_template.emplace_back(arg);
        }
    }

    // check for unsafe commands
    // TODO: this list can be extended, but forbid some dangerous ones for now
    const auto &cmd = _args_template.at(0);
    if (
        cmd == "rm" ||      // remove file
        cmd == "mv" ||      // move file
        cmd == "del" ||     // maybe some rm alias, but an actual command in PATH
        cmd == "rmdir" ||   // remove directory
        cmd == "find" ||    // the find command which can -exec commands
        cmd == "%f"         // the placeholder can not be executed
    ) {
        _command.clear();
        _args_template.clear();
        _command_contains_dangerous = true;
    }
}

const std::string PGSFrameCreator::commandTemplate(bool *is_dangerous) const
{
    std::string cmd;
    for (auto&& arg : _args_template)
    {
        cmd += arg + " ";
    }
    if (is_dangerous)
    {
        (*is_dangerous) = _command_contains_dangerous;
    }
    if (!cmd.empty())
    {
        cmd.erase(cmd.size(), cmd.size() - 2);
    }
    return cmd;
}
