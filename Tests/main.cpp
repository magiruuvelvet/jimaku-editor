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

    test("PngRenderer::render_simple", renderer_tests::render_simple, "test1.png", "ここがウチの村");
    test("PngRenderer::render_simple", renderer_tests::render_simple, "test2.png", "ここがウチの村\nのんびりのどかな所です");
    test("PngRenderer::render_simple", renderer_tests::render_simple, "test3.png", "夏休み");
    test("PngRenderer::render_simple", renderer_tests::render_simple, "test4.png", "（宮内{一穂|かずほ}）\nおばあちゃんが\n買ってくれたんだって");
    test("PngRenderer::render_simple", renderer_tests::render_simple, "test5.png", "（{宮内|みやうち}れんげ）おおーっ！");

    return has_failed_tests ? 1 : 0;
}
