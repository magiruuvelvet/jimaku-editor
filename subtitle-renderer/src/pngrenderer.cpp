#include "pngrenderer.hpp"

#define MAGICKCORE_QUANTUM_DEPTH 8
#define MAGICKCORE_HDRI_ENABLE 1
#include <Magick++.h>

#include "libs/lodepng/lodepng.hpp"

#include "helpers.hpp"

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

#include <cstring>

// TODO:
//  -> furigana-spacing
//  -> horizontal-numbers

// NOTES:
//  -> if furigana are too long, the image size may be too small for short text (example: {旭丘|あさひがおか})
//     can be easily fixed by adding extra spaces though

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

static const QFont compileFont(const std::string &fontFamily, unsigned long fontSize, const std::string &fontStyle)
{
    QFont font(fontFamily.c_str(), int(fontSize));

    if (fontStyle == "italic") {
        font.setItalic(true);
    } else if (fontStyle == "bold") {
        font.setBold(true);
    } else if (fontStyle == "bold-italic") {
        font.setBold(true);
        font.setItalic(true);
    }

    return font;
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
static void drawTextBorder(QPainter *painter, int x, int y, unsigned long borderSize, int width, int height, int alignment, const QString &text)
{
    // don't do anything when border size is zero
    if (borderSize == 0)
    {
        return;
    }

    for (auto b = 0U; b < borderSize + 1; ++b)
    {
        // draw last outer border with 50% transparency
        if (b == borderSize && b != 1)
        {
            painter->setOpacity(0.5);
        }

        // make signed for Qt
        const auto i = int(b);

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

    painter->setOpacity(1);
}

struct DrawnPosition
{
    QRect pos;
    bool halfwidth = false;
};

struct VerticalRenderingSettings
{
    QPoint pos;
    QSize size;
    bool isRotated = false;
};

static VerticalRenderingSettings verticalConfigurePainter(QPainter *painter, QString ch, int glyphWidth, int glyphHeight, int lineSpaceReduction, const QSize &imageSize, const DrawnPosition &lastPosition)
{
    painter->save();

    // keep track of rotation
    bool isRotated = false;

    // position offset
    QPoint pos{0, 0};

    // new size
    QSize size{glyphWidth, glyphHeight};

    // character rotation
    if (ch.contains(QRegularExpression("ー|（|）|「|」|｛|｝|＜|＞|─|〜|～|…|《|》")))
    {
        auto realHeight = lastPosition.halfwidth ? lastPosition.pos.height() / 2 : lastPosition.pos.height();
        auto p = QRect(
            lastPosition.pos.left(),
            lastPosition.pos.top() + glyphHeight - lineSpaceReduction, // note: some fonts may require +5 ??
            lastPosition.pos.width(),
            realHeight
        );

        painter->translate(p.center());
        painter->rotate(90);
        painter->translate(-p.center());
        isRotated = true;
    }

    // ASCII white space, reduce height by half of glyph height
    else if (ch == " ")
    {
        size = {0, (glyphHeight / 2)};
    }

    return {pos, size, isRotated};
}

} // anonymous namespace

PNGRenderer::PNGRenderer()
{
    init_qt_context();
}

PNGRenderer::PNGRenderer(const std::string &text, const std::string &fontFamily, unsigned long fontSize, unsigned long furiganaFontSize)
    : _text(text),
      _fontFamily(fontFamily),
      _fontSize(fontSize),
      _furiganaFontSize(furiganaFontSize)
{
    PNGRenderer();

    // if no font was given, use the default according to specs
    if (_fontFamily.empty())
    {
        _fontFamily = "TakaoPGothic";
    }
}

const std::vector<char> PNGRenderer::render(size_t *_size, pos_t *_pos, unsigned long *color_count) const
{
    const QString text = QString::fromUtf8(_text.c_str());
    const QFont font = compileFont(_fontFamily, _fontSize, _fontStyle);
    const QFont fontFurigana = compileFont(_fontFamily, _furiganaFontSize, _furiganaFontStyle);

    // split lines
    QStringList lines = text.split('\n', Qt::KeepEmptyParts);

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

    // image size for vertical rendering
    if (_vertical)
    {
        // fix image height to fit all Kanji
        size.setHeight((lineHeight * longestLineCount) - (_lineSpaceReduction * (longestLineCount - 1)) + 10);

        // fix image width
        size.setWidth(((glyphWidth + 30) * lines.size()));

        // increase width to fit Furigana
        for (auto&& line : lines)
        {
            bool hasFurigana = hasLineFurigana(line);
            if (hasFurigana)
            {
                size.setWidth(size.width() + furiGlyphWidth);
            }
        }
    }

    // create in-memory image
    QImage image(size, QImage::Format_RGBA8888_Premultiplied);
    QImage background(size, QImage::Format_RGBA8888_Premultiplied);
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
        if (_vertical)
        {
            int adjustX = 0;

            if (i != 0)
            {
                adjustX = 4;
            }

            int x = (size.width() - ((glyphWidth + adjustX) * (i + 1)) - 10) - nextXAdjust;
            int y = nextYAdjust;

            // Furigana on the right
            if (i == 0 && hasFurigana)
            {
                x -= int(furiGlyphWidth * 1.5);
                nextXAdjust = int(furiGlyphWidth * 1.5);
            }

            // Furigana on the left
            //  no X adjust from right needed

            // the position where QPainter has drawn the text (required to render Furigana later)
            QList<DrawnPosition> drawnPositions;

            // set main font
            painter.setFont(font);
            bgPainter.setFont(font);

            for (auto&& ch : splitIntoCharacters(lineWithoutFurigana))
            {
                auto height = mainMetrics.boundingRect(ch).height();

                // get last drawn position
                auto halfwidth = QChar(ch.at(0)) == QChar(0x20); // TODO: write function to handle halfwidth char detection
                auto lastDrawnPosition = drawnPositions.empty() ?
                    DrawnPosition{QRect(x, y - height + _lineSpaceReduction, glyphWidth, height), halfwidth} :
                    drawnPositions.last();

                // configure painter for vertical rendering
                auto bgSettings = verticalConfigurePainter(&bgPainter, ch, glyphWidth, height, _lineSpaceReduction, size, lastDrawnPosition);
                auto mainSettings = verticalConfigurePainter(&painter, ch, glyphWidth, height, _lineSpaceReduction, size, lastDrawnPosition);

                // draw text outline and shadow
                bgPainter.setPen(QColor(_borderColor.c_str()));
                drawTextBorder(&bgPainter, x - bgSettings.pos.x(), y - bgSettings.pos.y(), _borderSize, glyphWidth, bgSettings.size.height(), Qt::AlignCenter, ch);

                // draw main text
                QRect drawnPosition;
                painter.setPen(QColor(_fontColor.c_str()));
                painter.drawText(x - mainSettings.pos.x(), y - mainSettings.pos.y(), glyphWidth, mainSettings.size.height(), Qt::AlignCenter, ch, &drawnPosition);
                drawnPositions.append({drawnPosition, halfwidth});

                y += mainSettings.size.height() - _lineSpaceReduction;

                // set position when given
                if (_pos && !mainSettings.isRotated)
                {
                    if (_pos->x == 0 && _pos->y == 0)
                    {
                        _pos->vertical = true;
                        auto _x = drawnPosition.x() + glyphWidth;
                        auto _y = drawnPosition.y();
                        _pos->x = unsigned(_x < 0 ? 0: _x);
                        _pos->y = unsigned(_y < 0 ? 0: _y);
                    }
                }

                // reset painter to its previous state (saved by verticalConfigurePainter)
                painter.restore();
                bgPainter.restore();
            }

            // draw Furigana
            if (hasFurigana)
            {
                // set furigana font
                painter.setFont(fontFurigana);
                bgPainter.setFont(fontFurigana);

                for (auto&& f : furiganaPairs)
                {
                    // calculate starting positions for Furigana
                    auto startX = drawnPositions.at(f.startPos).pos.x() + drawnPositions.at(f.startPos).pos.width() + 5;

                    // center align vertically
                    auto topY = drawnPositions.at(f.startPos).pos.y();
                    auto bottomYPos = drawnPositions.at(f.startPos + f.length - 1).pos;
                    auto bottomY = bottomYPos.y() + bottomYPos.height();
                    auto centerHelper = bottomY - topY;
                    auto startY = topY + (centerHelper / 2) - (((f.furigana.size() * furiLineHeight) - (f.furigana.size() * _furiganaLineSpaceReduction)) / 2);

                    for (auto&& ch : splitIntoCharacters(f.furigana))
                    {
                        // draw on right
                        if (i == 0)
                        {
                            // draw text outline and shadow
                            bgPainter.setPen(QColor(_borderColor.c_str()));
                            drawTextBorder(&bgPainter, startX, startY, _furiganaBorderSize, furiGlyphWidth, furiLineHeight, Qt::AlignCenter, ch);

                            // draw main text
                            painter.setPen(QColor(_furiganaFontColor.c_str()));
                            painter.drawText(startX, startY, furiGlyphWidth, furiLineHeight, Qt::AlignCenter, ch);
                        }

                        // draw on left when multiple lines are present
                        if (i == lines.size() - 1 && lines.size() != 1)
                        {
                            // move to left
                            startX = drawnPositions.at(f.startPos).pos.x() - furiGlyphWidth - 5;

                            // draw text outline and shadow
                            bgPainter.setPen(QColor(_borderColor.c_str()));
                            drawTextBorder(&bgPainter, startX, startY, _furiganaBorderSize, furiGlyphWidth, furiLineHeight, Qt::AlignCenter, ch);

                            // draw main text
                            painter.setPen(QColor(_furiganaFontColor.c_str()));
                            painter.drawText(startX, startY, furiGlyphWidth, furiLineHeight, Qt::AlignCenter, ch);
                        }

                        // advance Y position
                        startY += furiLineHeight - _furiganaLineSpaceReduction;
                    }
                }
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

            // set position when given
            if (_pos)
            {
                _pos->vertical = false;

                if (_pos->x == 0 && lineWithoutFurigana.size() == longestLineCount)
                {
                    auto _x = drawnPosition.x();
                    _pos->x = unsigned(_x < 0 ? 0: _x);
                }

                if (_pos->y == 0 && i == lines.size() - 1)
                {
                    auto _y = drawnPosition.y() + drawnPosition.height();
                    _pos->y = unsigned(_y < 0 ? 0: _y);
                }
            }

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

    // end painting on background for manipulations
    bgPainter.end();

    // apply gaussian blur on background
    Magick::Image blurred(Magick::Blob(background.constBits(), std::size_t(size.width() * size.height() * 4)),
                          Magick::Geometry(std::size_t(size.width()), std::size_t(size.height())), 8, "RGBA");
    blurred.gaussianBlur(_gaussianBlurRadius, _gaussianBlurSigma);
    //blurred.shadow(); // saved for later
    Magick::Blob blurredData;
    blurred.write(&blurredData, "RGBA", 8);
    QImage bg(reinterpret_cast<const unsigned char*>(blurredData.data()), size.width(), size.height(), QImage::Format_RGBA8888_Premultiplied);
    background = bg;

    // merge main image into background so that it is in the foreground
    bgPainter.begin(&background);
    bgPainter.drawImage(0, 0, image);

    // reduce color count to be BDSup PGS compliant
    // limited to 255 colors per subtitle frame
    bgPainter.end();
    background.setColorCount(255);

    // Documentation: https://imagemagick.org/Magick++/Image++.html
    Magick::Image reduced(Magick::Blob(background.constBits(), std::size_t(size.width() * size.height() * 4)),
                          Magick::Geometry(std::size_t(size.width()), std::size_t(size.height())), 8, "RGBA");
    reduced.magick("RGBA");
    reduced.depth(8);

    // trim useless transparent border
    // allows more text to be stored into the 0xffff bytes limited PGS frame
    // needs recalculation of x,y pos for correct image placement
    auto cropFromTop = cropDetectionRow(&reduced, true);
    auto cropFromBottom = cropDetectionRow(&reduced, false);
    auto cropFromLeft = cropDetectionCol(&reduced, true);
    auto cropFromRight = cropDetectionCol(&reduced, false);
    reduced.crop(Magick::Geometry(reduced.size().width() - (cropFromLeft + cropFromRight),
                                  reduced.size().height() - (cropFromTop + cropFromBottom),
                                  cropFromLeft, cropFromTop));

    // adjust y position
    if (_pos)
    {
        if (_pos->y >= cropFromTop)
        {
            _pos->y -= cropFromTop;
        }
        else
        {
            _pos->y = 0;
        }
    }

    // adjust x position
    if (_pos)
    {
        if (_pos->x >= cropFromLeft)
        {
            _pos->x -= cropFromLeft;
        }
        else
        {
            _pos->x = 0;
        }
    }

    // max 255 allowed colors in PGSSUP palette, but reduce to configurable limit of colors
    reduced.quantizeColors(_colorLimit);

    // further optimize color palette to a bare minimum
    reduced.quantize();

    // strip useless data
    reduced.strip();

    // extract raw RGBA data from ImageMagick wrapped image
    Magick::Blob reducedData;
    reduced.write(&reducedData, "RGBA", 8);

    // count colors and create palette
    const auto pal = createPalette(reinterpret_cast<const unsigned char*>(reducedData.data()), reduced.size().width(), reduced.size().height());

    // lodepng is deleting things is should not delete, this is making std::vector<> unhappy :(
    unsigned char *pal_in = (unsigned char*) std::malloc(pal.size());
    unsigned char *pal_out = (unsigned char*) std::malloc(pal.size());
    std::memcpy(pal_in, pal.data(), pal.size());
    std::memcpy(pal_out, pal.data(), pal.size());
    // the above mess is deleted by lodepng

    // set image size when given
    if (_size)
    {
        _size->width = unsigned(reduced.size().width());
        _size->height = unsigned(reduced.size().height());
    }

    // set color count when given
    if (color_count)
    {
        // (*color_count) = reduced.colorMapSize(); <-- inaccurate color count (1~3 colors off)
        (*color_count) = pal.size() / 4;
    }

    // input color mode
    LodePNGColorMode input_mode;
    input_mode.bitdepth = 8;
    input_mode.colortype = LCT_RGBA;

    // output color mode
    LodePNGColorMode output_mode;
    output_mode.bitdepth = 8;
    output_mode.colortype = LCT_PALETTE;
    output_mode.palette = const_cast<unsigned char*>(pal.data());
    output_mode.palettesize = pal.size() / 4;
    auto out_bpp = lodepng_get_bpp(&output_mode);

    // allocate output buffer
    auto lode_out_size = (reduced.size().width() * reduced.size().height() * out_bpp + 7) / 8;
    unsigned char *lode_out = (unsigned char*) std::malloc(lode_out_size);

    // convert to palette mode
    auto res = lodepng_convert(lode_out, reinterpret_cast<const unsigned char*>(reducedData.data()),
                               &output_mode, &input_mode, reduced.size().width(), reduced.size().height());

    // check for conversion errors
    if (res != 0)
    {
        std::fprintf(stderr, "\nLodePNG error: %s\n", lodepng_error_text(res));

        // return ImageMagick PNG instead of Indexed PNG on error
        // note: ImageMagick can't handle semi-transparent pixels in 8-bit colormap PNG images

        Magick::Blob imageMagickPNG;
        reduced.write(&imageMagickPNG, "PNG", 8);

        const auto ptr = reinterpret_cast<const char*>(imageMagickPNG.data());
        return std::vector<char>(ptr, ptr + imageMagickPNG.length());
    }

    // encode 8-bit colormap PNG
    std::vector<unsigned char> png;

    // input pixel data
    lodepng::State state;
    state.info_raw.palette = pal_in;
    state.info_raw.palettesize = pal.size() / 4;
    state.info_raw.bitdepth = 8;
    state.info_raw.colortype = LCT_PALETTE;

    // output png config
    state.info_png.color.palette = pal_out;
    state.info_png.color.palettesize = pal.size() / 4;
    state.info_png.color.bitdepth = 8;
    state.info_png.color.colortype = LCT_PALETTE;

    // disable compression for speed, file size is width * height + palette + png sections
    state.encoder.zlibsettings.btype = 0;
    state.encoder.zlibsettings.use_lz77 = 0;
    state.encoder.zlibsettings.windowsize = 8;
    state.encoder.zlibsettings.nicematch = 8;
    state.encoder.zlibsettings.minmatch = 0;
    state.encoder.zlibsettings.lazymatching = 0;

    res = lodepng::encode(png, lode_out, reduced.size().width(), reduced.size().height(), state);

    // free output buffer
    std::free(lode_out);

    if (res != 0)
    {
        std::fprintf(stderr, "\nLodePNG error: %s\n", lodepng_error_text(res));

        // return ImageMagick PNG instead of Indexed PNG on error
        // note: ImageMagick can't handle semi-transparent pixels in 8-bit colormap PNG images

        Magick::Blob imageMagickPNG;
        reduced.write(&imageMagickPNG, "PNG", 8);

        const auto ptr = reinterpret_cast<const char*>(imageMagickPNG.data());
        return std::vector<char>(ptr, ptr + imageMagickPNG.length());
    }

    // return indexed 8-bit colormap PNG data
    return std::vector<char>(png.data(), png.data() + png.size());
}
