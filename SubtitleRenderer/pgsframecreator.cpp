#include "pgsframecreator.hpp"
#include "pngrenderer.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

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

bool PGSFrameCreator::render(const std::string &_out_path, bool verbose) const
{
    // check if out path exists and create it
    auto out_path = QString::fromUtf8(_out_path.c_str());
    QDir out(QString::fromUtf8(_out_path.c_str()));
    if (!out.exists())
    {
        if (!QDir::root().mkpath(out_path))
        {
            return false;
        }
    }

    // create and open definition_file file
    QFile definition_file(out_path + "/pgs.xml");
    if (!definition_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    QTextStream stream(&definition_file);

    // write command to run as xml comment
    stream << "<!-- command: pgssup -s " << _width << "x" << _height << " pgs.xml out.sup -->\n";

    // open xml segment
    stream << "<pgssup defaultoffset=\"0,0\">\n";
    stream.flush();

    // start processing all subtitles
    unsigned frameNo = 1;
    for (auto&& sub : _subtitles)
    {
        std::cout << "Rendering frame " << frameNo << "... ";

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
                x = _width - size.width - marginSide + (size.width - pos.x);
            }
            else if (alignment == "left")
            {
                x = marginSide;
            }
            else
            {
                // TODO: log warning
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
                // TODO: log warning
            }

            // align last main line above margin line and Furigana on bottom below margin
            y = _height - size.height - marginBottom + (size.height - pos.y);
        }

        if (verbose)
        {
            std::cout << " rendered image size = " << size.width << "x" << size.height << std::endl;
            std::cout << " calculated position = " << pos.x << "x" << pos.y << std::endl;
        }

        // write sub image to disk
        const auto filename = std::to_string(frameNo) + ".png";
        write(_out_path + "/" + filename, sub_image);

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

        ++frameNo;
    }

    // close xml segment
    stream << "</pgssup>\n";
    stream.flush();

    // close definition file
    definition_file.close();
    return true;
}
