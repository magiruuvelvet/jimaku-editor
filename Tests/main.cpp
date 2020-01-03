#include <iostream>
#include <stdexcept>
#include <string>

#include "srtparsertest.hpp"

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

    return has_failed_tests ? 1 : 0;
}
