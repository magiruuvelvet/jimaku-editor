#include <iostream>
#include <stdexcept>
#include <string>

#include "srtparsertest.hpp"
#include "renderertest.hpp"

static bool has_failed_tests = false;

template<typename Func, typename... Args>
void test(const std::string &test_name, Func func, Args&&... args)
{
    bool result = false;
    bool exception = false;
    std::string message;

    try {
        // execute test function and forward additional arguments
        result = func(args...);
    } catch (std::exception &e) {
        result = false;
        exception = true;
        message = e.what();
    }

    if (result)
    {
        std::cout << "[Test] " << test_name << ": PASS" << std::endl;
    }
    else
    {
        // at least one test failed
        has_failed_tests = true;

        if (exception)
        {
            std::cerr << "[Test] " << test_name << ": FAILED with exception -> " << message << std::endl;
        }
        else
        {
            std::cerr << "[Test] " << test_name << ": FAILED" << std::endl;
        }
    }
}

int main(void)
{
    std::cout << "Running tests..." << std::endl;

    test("SrtParser::parse_basic", srtparser_tests::parse_basic);
    test("SrtParser::parse_styled", srtparser_tests::parse_styled);

    // horizontal rendering tests
    test("PngRenderer::render_simple", renderer_tests::render_simple, "test1.png", "ここがウチの村", false);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "test2.png", "ここがウチの村\nのんびりのどかな所です", false);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "test3.png", "夏休み", false);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "test4.png", "（宮内{一穂|かずほ}）\nおばあちゃんが\n{買|か}ってくれたんだって", false);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "test5.png", "（{宮内|みやうち}れんげ）おおーっ！", false);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "test6.png", "戻ってないから\n行くよ 学校", false);

    test("PngRenderer::render_simple", renderer_tests::render_simple, "test7.png", "力を{集|あつ}め {新世界|しんせかい}への\nポータルを{開|ひら}く", false);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "test8.png", "{話|・}{せ|・}{る|・}", false);


    // vertical rendering tests
    test("PngRenderer::render_simple", renderer_tests::render_simple, "vtest1.png", "ここがウチの村", true);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "vtest2.png", "ここがウチの村\nのんびりのどかな所です", true);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "vtest3.png", "夏休み", true);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "vtest4.png", "（宮内{一穂|かずほ}）\nおばあちゃんが\n{買|か}ってくれたんだって", true);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "vtest5.png", "（{宮内|みやうち}れんげ）おおーっ！", true);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "vtest6.png", "戻ってないから\n行くよ 学校", true);

    test("PngRenderer::render_simple", renderer_tests::render_simple, "vtest7.png", "力を{集|あつ}め {新世界|しんせかい}への\nポータルを{開|ひら}く", true);
    test("PngRenderer::render_simple", renderer_tests::render_simple, "vtest8.png", "{話|・}{せ|・}{る|・}", true);

    return has_failed_tests ? 1 : 0;
}
