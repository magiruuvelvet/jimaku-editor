#include <iostream>
#include <fstream>

#include <renderer/pngrenderer.hpp>
#include <renderer/pgsframecreator.hpp>

namespace renderer_tests {

bool render_simple(const std::string &out_file, const std::string &text, bool vertical)
{
    PNGRenderer renderer(text, "TakaoPGothic");
    //renderer.setTextJustify(PNGRenderer::TextJustify::Left);
    //renderer.setFuriganaDistance(PNGRenderer::FuriganaDistance::Narrow);
    //renderer.setLineSpaceReduction(27);
    //renderer.setFuriganaLineSpaceReduction(10);
    //renderer.setFuriganaLineSpaceReduction(-3);
    renderer.setFontSize(42);

    if (vertical)
    {
        renderer.setVertical(true);
    }

    PNGRenderer::size_t size;
    PNGRenderer::pos_t pos;

    const auto png = renderer.render(&size, &pos);

    std::printf("[%s] %ux%u @ %ux%u\n", out_file.c_str(), size.width, size.height, pos.x, pos.y);

    const auto png_file = std::string{UNIT_TEST_TEMPORARY_DIR} + "/" + out_file;

    // write png to disk
    std::ofstream file(png_file, std::ios::binary);
    file.write(png.data(), unsigned(png.size()));
    file.close();

    return !png.empty();
}

bool render_pgs_frames()
{
    const auto srt_file = std::string{UNIT_TEST_CURRENT_DIR} + "/test_custom.ja.srt";
    const auto subs = SrtParser::parseStyled(srt_file);

    const auto out_path = std::string{UNIT_TEST_TEMPORARY_DIR} + "/pgs";

    PGSFrameCreator fc(subs, subs.at(0).width(), subs.at(0).height());

    return fc.render(out_path) == PGSFrameCreator::Success;
}

} // namespace renderer_tests
