#include <iostream>
#include <stdexcept>
#include <string>

#include "srtparsertest.hpp"

template<typename Func>
void test(const std::string &test_name, Func func)
{
    bool result = false;
    bool exception = false;
    std::string message;

    try {
        result = func();
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

    return 0;
}
