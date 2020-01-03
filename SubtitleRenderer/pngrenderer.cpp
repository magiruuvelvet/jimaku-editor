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

PNGRenderer::PNGRenderer()
{
    init_qt_context();
}

PNGRenderer::PNGRenderer(const std::string &text, const std::string &fontFamily, int fontSize)
    : _text(text),
      _fontFamily(fontFamily),
      _fontSize(fontSize)
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

    // split lines
    QStringList lines = text.split('\n', QString::KeepEmptyParts);

    // calculate necessary size for subtitle image
    QSize size{0, 0};
    int lineHeight = 0;

    for (auto&& line : lines)
    {
        QFontMetrics textMetrics(font);
        auto rect = textMetrics.boundingRect(line);

        // bounding rect has not enough width, use another function for this
        rect.setWidth(textMetrics.horizontalAdvance(line));

        // maximum width
        if (rect.size().width() > size.width())
        {
            size.setWidth(rect.width());
        }

        // maximum height
        if (rect.height() > size.height())
        {
            size.setHeight(rect.height());
            lineHeight = rect.height();
        }
    }

    // line space reducer
    static int line_space_reducer = 10;

    // calculate required image height
    size.setHeight((size.height() * lines.size()) - (line_space_reducer * lines.size()));

    // create in-memory image
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    // paint text onto image
    QPainter painter(&image);
    painter.setFont(font);
    painter.setPen(Qt::black);

    for (auto i = 0; i < lines.size(); ++i)
    {
        int y = -line_space_reducer;

        if (i != 0)
        {
            y = ((lineHeight / 2) + line_space_reducer) * i;
        }

        // the Y position is reduced intentionally to not have extreme spacing between lines
        // Japanese subtitles are more compact to be easier to read
        painter.drawText(0, y, size.width(), lineHeight, Qt::AlignCenter, lines.at(i));
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
