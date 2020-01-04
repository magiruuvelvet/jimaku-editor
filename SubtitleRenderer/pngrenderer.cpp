#include "pngrenderer.hpp"

#include <QGuiApplication>
#include <QPaintDevice>
#include <QPainter>
#include <QImage>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QFontInfo>
#include <QFont>
#include <QBuffer>
#include <QRegularExpression>

#include <QDebug>

static QGuiApplication *qt_app = nullptr;

static void init_qt_context()
{
    if (!qt_app)
    {
        static int arg = 0;
        // must be called, otherwise gui-based functions just segfault
        qt_app = new QGuiApplication(arg, nullptr);
    }
}

namespace  {

struct FuriganaPair
{
    QString kanji;
    QString furigana;
    int startPos = -1;
    int length = 0;
};

static const QString getLineWithoutFurigana(const QString &line, QList<FuriganaPair> *furiganaPairs = nullptr)
{
    // matches {漢字|かんじ} non-greedy and creates matching groups
    static const QRegularExpression furiganaCapture(R"(\{(.*?)\|(.*?)\})");

    QString newLine = line.split(furiganaCapture).join("|");

    QList<QRegularExpressionMatch> lastMatchs;
    int lastIndex = 0;
    auto matches = furiganaCapture.globalMatch(line);
    while (matches.hasNext())
    {
        auto match = matches.next();

        lastIndex = newLine.indexOf('|', lastIndex);

        // replace pipe symbol with original kanji
        if (lastIndex != -1)
        {
            newLine.remove(lastIndex, 1);
            newLine.insert(lastIndex, match.capturedTexts().at(1));
            lastIndex = 0;
        }

        // append to furigana pair list
        if (furiganaPairs)
        {
            // fix start offset for next match because of string manipulation
            int startOffset = 0;
            if (!lastMatchs.isEmpty() && lastMatchs.last().hasMatch())
            {
                for (auto&& m : lastMatchs)
                {
                    // offset is the total of the capture length "{}" subtracted with the kanji count
                    // all past matches are summed up as the new string shrinks to the left
                    startOffset += m.capturedLength(0) - m.capturedLength(1);
                }
            }

            furiganaPairs->append({
                match.capturedTexts().at(1),          // original kanji
                match.capturedTexts().at(2),          // furigana
                match.capturedStart(0) - startOffset, // position in line where kanji with furigana starts
                match.capturedLength(1),              // kanji count
            });
        }

        // store last match, required to fix the start offset
        lastMatchs.append(match);
    }

    return newLine;
}

static bool hasLineFurigana(const QString &line)
{
    QList<FuriganaPair> furiganaPairs;
    getLineWithoutFurigana(line, &furiganaPairs);
    return !furiganaPairs.empty();
}

static Qt::AlignmentFlag getQtTextAlignmentFlag(PNGRenderer::TextJustify textJustify)
{
    if (textJustify == PNGRenderer::TextJustify::Left)
    {
        return Qt::AlignLeft;
    }
    else if (textJustify == PNGRenderer::TextJustify::Center)
    {
        return Qt::AlignCenter;
    }
    else if (textJustify == PNGRenderer::TextJustify::Right)
    {
        return Qt::AlignRight;
    }
    else
    {
        return Qt::AlignCenter;
    }
}

static int getFuriganaDistanceValue(PNGRenderer::FuriganaDistance furiganaDistance)
{
    if (furiganaDistance == PNGRenderer::FuriganaDistance::None)
    {
        // absolutely no spacing between Kanji and Furigana
        // (may look better in some cases than Narrow, best to be judged
        //  by the subtitle writer)
        return 15;
    }
    else if (furiganaDistance == PNGRenderer::FuriganaDistance::Narrow)
    {
        // slight spacing between Kanji and Furigana
        // (the default, works best for most cases)
        return 20;
    }
    else if (furiganaDistance == PNGRenderer::FuriganaDistance::Far)
    {
        // huge spacing between Kanji and Furigana
        // (may has its use cases, hence its an offered option)
        // (note: some fonts may need this)
        return 25;
    }
    else if (furiganaDistance == PNGRenderer::FuriganaDistance::Unchanged)
    {
        // for use with fonts without extra top and bottom spacing
        return -1;
    }
    else
    {
        return 20;
    }
}

} // anonymous namespace

PNGRenderer::PNGRenderer()
{
    init_qt_context();
}

PNGRenderer::PNGRenderer(const std::string &text, const std::string &fontFamily, int fontSize, int furiganaFontSize)
    : _text(text),
      _fontFamily(fontFamily),
      _fontSize(fontSize),
      _furiganaFontSize(furiganaFontSize)
{
    PNGRenderer();

    // if no font was given, use the default system font according to Qt
    if (_fontFamily.empty())
    {
        _fontFamily = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family().toUtf8().constData();
    }
}

//
// first draft and experiments to test rendering capabilities of Qt
// first result for horizontal text rendering is looking good :)
//
//
const std::vector<char> PNGRenderer::render() const
{
    const QString text = QString::fromUtf8(_text.c_str());
    const QFont font = QFont(_fontFamily.c_str(), _fontSize);
    const QFont fontFurigana = QFont(_fontFamily.c_str(), _furiganaFontSize);

    // split lines
    QStringList lines = text.split('\n', QString::KeepEmptyParts);

    // calculate necessary size for subtitle image
    QSize size{0, 0};
    int lineHeight = 0, furiLineHeight = 0;

    QFontMetrics mainMetrics(font);
    QFontMetrics furiMetrics(fontFurigana);

    for (auto&& line : lines)
    {
        // take metrics without Furigana
        auto lineWithoutFurigana = getLineWithoutFurigana(line);

        auto mainRect = mainMetrics.boundingRect(lineWithoutFurigana);
        auto furiRect = furiMetrics.boundingRect(line);

        // bounding rect may not have enough width, use another function for this
        mainRect.setWidth(mainMetrics.horizontalAdvance(lineWithoutFurigana));

        // maximum width
        if (mainRect.size().width() > size.width())
        {
            size.setWidth(mainRect.width());
        }

        // maximum height
        if (mainRect.height() > size.height())
        {
            size.setHeight(mainRect.height());
            lineHeight = mainRect.height();
        }

        // maximum Furigana height
        if (furiRect.height() > furiLineHeight)
        {
            furiLineHeight = furiRect.height();
        }
    }

    // calculate required image height
    size.setHeight((size.height() * lines.size()) - (_lineSpaceReduction * (lines.size() - 1)));

    // increase height to fit Furigana
    for (auto&& line : lines)
    {
        bool hasFurigana = hasLineFurigana(line);
        if (hasFurigana)
        {
            size.setHeight(size.height() + furiLineHeight);
        }
    }

    // create in-memory image
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    // paint text onto image
    QPainter painter(&image);
    painter.setFont(font);
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    // horizontal only
    int nextYAdjust = 0;

    // determine text alignment
    Qt::AlignmentFlag alignment = getQtTextAlignmentFlag(_textJustify);

    // get furigana distance value
    int furiganaDistance = getFuriganaDistanceValue(_furiganaDistance);

    for (auto i = 0; i < lines.size(); ++i)
    {
        QList<FuriganaPair> furiganaPairs;
        auto lineWithoutFurigana = getLineWithoutFurigana(lines.at(i), &furiganaPairs);
        bool hasFurigana = !furiganaPairs.isEmpty();

        // vertical rendering
        if (_vertical)
        {
            // TODO
        }

        // horizontal rendering
        else
        {
            // the Y position is reduced intentionally to not have extreme spacing between lines
            // Japanese subtitles are more compact to be easier to read

            int y = nextYAdjust;

            if (i != 0)
            {
                y = (lineHeight * i) - (_lineSpaceReduction * i) + nextYAdjust;
            }

            // Furigana on top
            if (i == 0 && hasFurigana)
            {
                y += furiLineHeight;
                nextYAdjust = furiLineHeight;
            }

            // Furigana on bottom
            //  no Y adjust from top needed

            // the position where QPainter has drawn the text (required to render Furigana later)
            QRect drawnPosition;

            // set main font
            painter.setFont(font);

            // TODO: draw text outline and shadow
            painter.setPen(Qt::black);
            painter.drawText(0, y, size.width(), lineHeight, alignment, lineWithoutFurigana, &drawnPosition);

            // draw main text
            painter.setPen(Qt::white);
            painter.drawText(0, y, size.width(), lineHeight, alignment, lineWithoutFurigana, &drawnPosition);

            // draw Furigana
            if (hasFurigana)
            {
                // set furigana font
                painter.setFont(fontFurigana);

                for (auto&& f : furiganaPairs)
                {
                    // get real width of Kanji and Furigana
                    auto kanjiWidth = mainMetrics.horizontalAdvance(f.kanji);
                    auto furiWidth = furiMetrics.horizontalAdvance(f.furigana);

                    // calculate starting position for Furigana
                    auto startX = mainMetrics.horizontalAdvance(lineWithoutFurigana, f.startPos);
                    // center align Furigana
                    startX += (kanjiWidth / 2) - (furiWidth / 2);
                    // adjust position where text was actually drawn by QPainter
                    startX += drawnPosition.x();

                    // calculate distance based on line height when mode is "Unchanged"
                    if (furiganaDistance == -1)
                    {
                        furiganaDistance = lineHeight / 2;
                    }

                    // draw on top
                    if (i == 0)
                    {
                        // TODO: draw text outline and shadow
                        painter.setPen(Qt::black);
                        painter.drawText(startX, y - furiganaDistance, furiWidth, furiLineHeight, 0, f.furigana);

                        // draw main text
                        painter.setPen(Qt::white);
                        painter.drawText(startX, y - furiganaDistance, furiWidth, furiLineHeight, 0, f.furigana);
                    }

                    // draw on bottom when multiple lines are present
                    if (i == lines.size() - 1 && lines.size() != 1)
                    {
                        // TODO: draw text outline and shadow
                        painter.setPen(Qt::black);
                        painter.drawText(startX, y + furiganaDistance + (lineHeight / 2), furiWidth, furiLineHeight, 0, f.furigana);

                        // draw main text
                        painter.setPen(Qt::white);
                        painter.drawText(startX, y + furiganaDistance + (lineHeight / 2), furiWidth, furiLineHeight, 0, f.furigana);
                    }
                }
            }
        }
    }

    // write PNG data and return it
    QByteArray png_data;
    QBuffer png_data_buf(&png_data);
    png_data_buf.open(QIODevice::WriteOnly);

    image.save(&png_data_buf, "PNG");

    // convert Qt data to STL
    std::vector<char> png_data_no_qt;
    for (auto&& d : png_data)
    {
        png_data_no_qt.emplace_back(d);
    }

    return png_data_no_qt;
}
