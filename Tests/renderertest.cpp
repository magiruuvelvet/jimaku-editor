#include <iostream>
#include <fstream>

#include <pngrenderer.hpp>

namespace renderer_tests {

bool render_simple(const std::string &out_file, const std::string &text)
{
    PNGRenderer renderer(text, "Noto Sans CJK JP");
    //renderer.setTextJustify(PNGRenderer::TextJustify::Left);
    const auto png = renderer.render();

    const auto png_file = std::string{UNIT_TEST_TEMPORARY_DIR} + "/" + out_file;

    // write png to disk
    std::ofstream file(png_file, std::ios::binary);
    file.write(png.data(), png.size());
    file.close();

    return !png.empty();
}

} // namespace renderer_tests
