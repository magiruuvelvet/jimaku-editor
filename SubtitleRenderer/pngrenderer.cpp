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
    static const QRegularExpression furiganaCapture(R"(\{(.*)\|(.*)\})");

    QString newLine = line.split(furiganaCapture).join("|");

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
        }

        // append to furigana pair list
        if (furiganaPairs)
        {
            furiganaPairs->append({
                match.capturedTexts().at(1),    // original kanji
                match.capturedTexts().at(2),    // furigana
                match.capturedStart(0),         // position in line where kanji with furigana starts
                match.capturedLength(1),        // kanji count
            });
        }
    }

    return newLine;
}

static bool hasLineFurigana(const QString &line)
{
    QList<FuriganaPair> furiganaPairs;
    getLineWithoutFurigana(line, &furiganaPairs);
    return !furiganaPairs.empty();
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

    // line space reducer
    static int line_space_reducer = 20;

    // calculate required image height
    size.setHeight((size.height() * lines.size()) - (line_space_reducer * (lines.size() - 1)));

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

    int nextYAdjust = 0;

    // determine text alignment
    Qt::AlignmentFlag alignment = Qt::AlignCenter;
    if (_textJustify == TextJustify::Left)
    {
        alignment = Qt::AlignLeft;
    }
    else if (_textJustify == TextJustify::Center)
    {
        alignment = Qt::AlignCenter;
    }
    else if (_textJustify == TextJustify::Right)
    {
        alignment = Qt::AlignRight;
    }

    for (auto i = 0; i < lines.size(); ++i)
    {
        // vertical rendering
        if (_vertical)
        {
            // TODO
        }

        // horizontal rendering
        else
        {
            QList<FuriganaPair> furiganaPairs;
            auto lineWithoutFurigana = getLineWithoutFurigana(lines.at(i), &furiganaPairs);
            bool hasFurigana = hasLineFurigana(lines.at(i));

            // the Y position is reduced intentionally to not have extreme spacing between lines
            // Japanese subtitles are more compact to be easier to read

            int y = -5 + nextYAdjust;

            if (i != 0)
            {
                y = (lineHeight * i) - (line_space_reducer * i) - 5 + nextYAdjust;
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

                    // draw on top
                    if (i == 0)
                    {
                        // TODO: draw text outline and shadow
                        painter.setPen(Qt::black);
                        painter.drawText(startX, 12, furiWidth, furiLineHeight, 0, f.furigana);

                        // draw main text
                        painter.setPen(Qt::white);
                        painter.drawText(startX, 12, furiWidth, furiLineHeight, 0, f.furigana);
                    }

                    // draw on bottom
                    if (i == lines.size() - 1)
                    {
                        // TODO
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
