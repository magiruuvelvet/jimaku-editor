#include "pgsframecreator.hpp"
#include "pngrenderer.hpp"

#include <fstream>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>

#include <QImage>
#include <QPainter>

using SrtParser::StyledSubtitleItem;

namespace {

static SrtParser::timestamp_t pgsTimestamp(SrtParser::timestamp_t ms)
{
    // PGS timestamps have an accuracy of 90 kHz
    return ms * 90;
}

static void write(const std::string &filename, const std::vector<char> &data)
{
    // write png to disk
    std::ofstream file(filename, std::ios::binary);
    file.write(data.data(), data.size());
    file.close();
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

bool PGSFrameCreator::render(const std::string &_out_path) const
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

    // create and open timing file
    QFile timing_file(out_path + "/pgs.script");
    if (!timing_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    QTextStream stream(&timing_file);

    // render empty image for first subtitle frame (work around a PGS encoder bug)
    PNGRenderer renderer;
    const auto first_image = renderer.render();
    write(_out_path + "/" + "0.png", first_image);
    stream << 0 << ' ' << 0 << ' ' << 0 << ' ' << "0.png" << '\n';

    // start processing all subtitles
    unsigned frameNo = 1;
    for (auto&& sub : _subtitles)
    {
        // setup renderer
        PNGRenderer renderer(sub.text(), sub.property(StyledSubtitleItem::FontFamily),
                             sub.fontSize(), sub.furiganaFontSize());

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

        // render subtitle image
        PNGRenderer::size_t size;
        PNGRenderer::pos_t pos;
        const auto sub_image = renderer.render(&size, &pos);

        // create frame
        //QImage frame(int(_width), int(_height), QImage::Format_ARGB32);
        //frame.fill(Qt::transparent);

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

        // write sub image to disk
        const auto filename = std::to_string(frameNo) + ".png";
        write(_out_path + "/" + filename, sub_image);

        // update timing file
        //   >>> timestamp x y filename
        stream << pgsTimestamp(sub.startTime()) << ' ' << x << ' ' << y << ' ' << filename.c_str() << '\n';
        stream << pgsTimestamp(sub.endTime()) << ' ' << 0 << ' ' << 0 << ' ' << "clean" << '\n';

        ++frameNo;
    }

    // close timing file
    timing_file.close();
    return true;
}
