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

// TODO:
//  furigana-spacing
//  horizontal-numbers

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

static QStringList splitIntoCharacters(const QString &str)
{
    QStringList chars;
    for (auto&& c : str)
    {
        chars.append(c);
    }
    return chars;
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

// QPainter can't do this apparently, so we need to brute force it instead :(
static void drawTextBorder(QPainter *painter, int x, int y, int borderSize, int width, int height, int alignment, const QString &text)
{
    for (auto i = 0; i < borderSize + 1; ++i)
    {
        // top left
        painter->drawText(x-i, y-i, width, height, alignment, text);
        // top
        painter->drawText(x, y-i, width, height, alignment, text);
        // top right
        painter->drawText(x+i, y-i, width, height, alignment, text);
        // left
        painter->drawText(x-i, y, width, height, alignment, text);
        // middle
        painter->drawText(x, y, width, height, alignment, text);
        // right
        painter->drawText(x+i, y, width, height, alignment, text);
        // bottom left
        painter->drawText(x-i, y+i, width, height, alignment, text);
        // bottom
        painter->drawText(x, y+i, width, height, alignment, text);
        // bottom right
        painter->drawText(x+i, y+i, width, height, alignment, text);
    }
}

struct VerticalRenderingSettings
{
    QPoint pos;
    QSize size;
};

// TODO: *** QPainter::rotate !!! (needs more handling to position the chars correctly)
static VerticalRenderingSettings verticalConfigurePainter(QPainter *painter, QString ch, int glyphWidth, int glyphHeight)
{
    painter->save();

    QPoint pos{0, 0};
    QSize size{0, 0};

    if (ch.contains(QRegularExpression("（|）|「|」|ー")))
    {
        painter->rotate(90);
        // literally only works for "ー" :(
        pos = {-10, (glyphHeight * 2) + 35};
    }

    return {pos, size};
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
    int longestLineCount = 0;
    int lineHeight = 0, furiLineHeight = 0;
    int glyphWidth = 0, furiGlyphWidth = 0;

    QFontMetrics mainMetrics(font);
    QFontMetrics furiMetrics(fontFurigana);

    for (auto&& line : lines)
    {
        // take metrics without Furigana
        auto lineWithoutFurigana = getLineWithoutFurigana(line);

        if (lineWithoutFurigana.size() > longestLineCount)
        {
            longestLineCount = lineWithoutFurigana.size();
        }

        auto mainRect = mainMetrics.boundingRect(lineWithoutFurigana);
        auto furiRect = furiMetrics.boundingRect(line);

        for (auto&& c : lineWithoutFurigana)
        {
            auto glyphRect = mainMetrics.boundingRect(c);
            auto furiGlyphRect = furiMetrics.boundingRect(c);

            // maximum glyph width
            if (glyphRect.size().width() > glyphWidth)
            {
                glyphWidth = glyphRect.size().width();
            }

            // maximum Furigana glyph width
            if (furiGlyphRect.size().width() > furiGlyphWidth)
            {
                furiGlyphWidth = furiGlyphRect.size().width();
            }
        }

        // bounding rect may not have enough width, use another function for this
        mainRect.setWidth(mainMetrics.horizontalAdvance(lineWithoutFurigana) + 10);

        // maximum total width
        if (mainRect.size().width() > size.width())
        {
            size.setWidth(mainRect.width());
        }

        // maximum total height
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
    size.setHeight((size.height() * lines.size()) - (_lineSpaceReduction * (lines.size() - 1)) + 10);

    // increase height to fit Furigana
    for (auto&& line : lines)
    {
        bool hasFurigana = hasLineFurigana(line);
        if (hasFurigana)
        {
            size.setHeight(size.height() + furiLineHeight);
        }
    }

    // invert the image size on vertical rendering
    if (_vertical)
    {
        size = QSize(size.height(), size.width());

        // fix image height to fit all Kanji
        size.setHeight(((lineHeight + 2) * longestLineCount) - (_lineSpaceReduction * (lines.size() - 1)));
    }

    // create in-memory image
    QImage image(size, QImage::Format_ARGB32);
    QImage background(size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    background.fill(Qt::transparent);

    // paint text onto image
    QPainter painter(&image);
    painter.setFont(font);
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    QPainter bgPainter(&background);
    bgPainter.setFont(font);
    bgPainter.setBackgroundMode(Qt::TransparentMode);
    bgPainter.setRenderHint(QPainter::Antialiasing, true);
    bgPainter.setRenderHint(QPainter::TextAntialiasing, true);

    // determine text alignment
    Qt::AlignmentFlag alignment = getQtTextAlignmentFlag(_textJustify);

    if (_vertical)
    {
        alignment = Qt::AlignCenter;
    }

    // get furigana distance value
    int furiganaDistance = getFuriganaDistanceValue(_furiganaDistance);

    // alignment helper
    int nextXAdjust = alignment == Qt::AlignCenter ? 0 : 5;
    int nextYAdjust = 5;

    for (auto i = 0; i < lines.size(); ++i)
    {
        QList<FuriganaPair> furiganaPairs;
        auto lineWithoutFurigana = getLineWithoutFurigana(lines.at(i), &furiganaPairs);
        bool hasFurigana = !furiganaPairs.isEmpty();

        // vertical rendering
        // TODO
        if (_vertical)
        {
            int adjustX = 0;

            if (i != 0)
            {
                adjustX = 4;
            }

            int x = (size.width() - ((glyphWidth + adjustX) * (i + 1)) - 10) - nextXAdjust;
            int y = 0;

            // Furigana on the right
            if (i == 0 && hasFurigana)
            {
                x -= furiGlyphWidth * 1.5;
                nextXAdjust = furiGlyphWidth * 1.5;
            }

            std::printf("x=%i\n", x);

            // Furigana on the left
            //  no X adjust from right needed

            // the position where QPainter has drawn the text (required to render Furigana later)
            QList<QRect> drawnPositions;

            // set main font
            painter.setFont(font);
            bgPainter.setFont(font);

            int chCounter = 1;
            for (auto&& ch : splitIntoCharacters(lineWithoutFurigana))
            {
                auto height = mainMetrics.boundingRect(ch).height();
                std::printf("%s = %i\n", QString(ch).toUtf8().constData(), height);

                auto bgSettings = verticalConfigurePainter(&bgPainter, ch, glyphWidth, height);
                auto mainSettings = verticalConfigurePainter(&painter, ch, glyphWidth, height);

                // draw text outline and shadow
                bgPainter.setPen(QColor(_borderColor.c_str()));
                drawTextBorder(&bgPainter, x - mainSettings.pos.x(), y - mainSettings.pos.y(), _borderSize, glyphWidth, height, Qt::AlignCenter, ch);

                // draw main text
                QRect drawnPosition;
                painter.setPen(QColor(_fontColor.c_str()));
                painter.drawText(x - bgSettings.pos.x(), y - bgSettings.pos.y(), glyphWidth, height, Qt::AlignCenter, ch, &drawnPosition);
                drawnPositions.append(drawnPosition);

                y += height - _lineSpaceReduction;
                ++chCounter;

                // reset painter to its previous state (saved by verticalConfigurePainter)
                painter.restore();
                bgPainter.restore();
            }
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
                nextYAdjust += furiLineHeight;
            }

            // Furigana on bottom
            //  no Y adjust from top needed

            // the position where QPainter has drawn the text (required to render Furigana later)
            QRect drawnPosition;

            // set main font
            painter.setFont(font);
            bgPainter.setFont(font);

            // draw text outline and shadow
            bgPainter.setPen(QColor(_borderColor.c_str()));
            drawTextBorder(&bgPainter, nextXAdjust, y, _borderSize, size.width(), lineHeight, alignment, lineWithoutFurigana);

            // draw main text
            painter.setPen(QColor(_fontColor.c_str()));
            painter.drawText(nextXAdjust, y, size.width(), lineHeight, alignment, lineWithoutFurigana, &drawnPosition);

            // draw Furigana
            if (hasFurigana)
            {
                // set furigana font
                painter.setFont(fontFurigana);
                bgPainter.setFont(fontFurigana);

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

                    auto realDistance = furiganaDistance;

                    // draw on top
                    if (i == 0)
                    {
                        // calculate distance based on line height when mode is "Unchanged"
                        if (furiganaDistance == -1)
                        {
                            realDistance = lineHeight / 2;
                        }

                        // draw text outline and shadow
                        bgPainter.setPen(QColor(_borderColor.c_str()));
                        drawTextBorder(&bgPainter, startX, y - realDistance, _furiganaBorderSize, furiWidth, furiLineHeight, 0, f.furigana);

                        // draw main text
                        painter.setPen(QColor(_furiganaFontColor.c_str()));
                        painter.drawText(startX, y - realDistance, furiWidth, furiLineHeight, 0, f.furigana);
                    }

                    // draw on bottom when multiple lines are present
                    if (i == lines.size() - 1 && lines.size() != 1)
                    {
                        // calculate distance based on line height when mode is "Unchanged"
                        if (furiganaDistance == -1)
                        {
                            realDistance = 0;
                        }

                        auto dY = (y + lineHeight) - realDistance;

                        // draw text outline and shadow
                        bgPainter.setPen(QColor(_borderColor.c_str()));
                        drawTextBorder(&bgPainter, startX, dY, _furiganaBorderSize, furiWidth, furiLineHeight, 0, f.furigana);

                        // draw main text
                        painter.setPen(QColor(_furiganaFontColor.c_str()));
                        painter.drawText(startX, dY, furiWidth, furiLineHeight, 0, f.furigana);
                    }
                }
            }
        }
    }

    // TODO: blur background image
    // current function can't handle transparency correctly
    //bgPainter.end();
    //background = blurImage(&background);

    // merge main image into background so that it is in the foreground
    //bgPainter.begin(&background);
    bgPainter.drawImage(0, 0, image);

    // write PNG data and return it
    QByteArray png_data;
    QBuffer png_data_buf(&png_data);
    png_data_buf.open(QIODevice::WriteOnly);

    background.save(&png_data_buf, "PNG");

    // convert Qt data to STL
    std::vector<char> png_data_no_qt;
    for (auto&& d : png_data)
    {
        png_data_no_qt.emplace_back(d);
    }

    return png_data_no_qt;
}
