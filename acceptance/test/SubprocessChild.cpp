#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
    std::size_t ParseSize(const char* value)
    {
        return static_cast<std::size_t>(std::stoull(value));
    }

    int ParseExitCode(const char* value)
    {
        return std::stoi(value);
    }
}

int main(int argc, char** argv)
{
    std::string stdoutText;
    std::string stderrText;
    std::size_t stdoutBytes = 0;
    std::size_t stderrBytes = 0;
    int exitCode = 0;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg{ argv[i] };

        if (arg == "--stdout" && i + 1 < argc)
        {
            stdoutText += argv[++i];
            continue;
        }

        if (arg == "--stderr" && i + 1 < argc)
        {
            stderrText += argv[++i];
            continue;
        }

        if (arg == "--out-bytes" && i + 1 < argc)
        {
            stdoutBytes = ParseSize(argv[++i]);
            continue;
        }

        if (arg == "--err-bytes" && i + 1 < argc)
        {
            stderrBytes = ParseSize(argv[++i]);
            continue;
        }

        if (arg == "--exit" && i + 1 < argc)
        {
            exitCode = ParseExitCode(argv[++i]);
            continue;
        }

        throw std::runtime_error("Invalid arguments");
    }

    std::cout << stdoutText;
    for (std::size_t i = 0; i < stdoutBytes; ++i)
        std::cout.put('O');

    std::cerr << stderrText;
    for (std::size_t i = 0; i < stderrBytes; ++i)
        std::cerr.put('E');

    std::cout.flush();
    std::cerr.flush();

    return exitCode;
}
