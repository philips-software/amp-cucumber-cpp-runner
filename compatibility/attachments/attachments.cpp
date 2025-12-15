#include "cucumber_cpp/CucumberCpp.hpp"
#include "library/engine/ExecutionContext.hpp"
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <sstream>
#include <string>

namespace
{
    const std::filesystem::path currentCompileDir = std::filesystem::path{ std::source_location::current().file_name() }.parent_path();
}

WHEN(R"(the string {string} is attached as {string})", (const std::string& text, const std::string& mediaType))
{
    Attach(text, mediaType);
}

WHEN(R"(the string {string} is logged)", (const std::string& text))
{
    Log(text);
}

WHEN(R"(text with ANSI escapes is logged)")
{
    Log("This displays a \x1b[31mr\x1b[0m\x1b[91ma\x1b[0m\x1b[33mi\x1b[0m\x1b[32mn\x1b[0m\x1b[34mb\x1b[0m\x1b[95mo\x1b[0m\x1b[35mw\x1b[0m");
}

WHEN(R"(the following string is attached as {string}:)", (const std::string& mediaType))
{
    Attach(docString->content, mediaType);
}

WHEN(R"(an array with {int} bytes is attached as {string})", (std::int32_t size, const std::string& mediaType))
{
    std::string data(size, '\0');
    std::iota(data.begin(), data.end(), 0);
    std::stringstream stream{ data };
    Attach(stream, mediaType);
}

WHEN(R"(a PDF document is attached and renamed)")
{
    std::ifstream pdfFile{ currentCompileDir / "document.pdf", std::ios::binary };
    Attach(pdfFile, cucumber_cpp::library::engine::AttachOptions{ "application/pdf", "renamed.pdf" });
}

WHEN(R"(a link to {string} is attached)", (const std::string& url))
{
    Link(url);
}
