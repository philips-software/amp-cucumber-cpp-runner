#include "cucumber-cpp/Rtrim.hpp"
#include <algorithm>
#include <cctype>
#include <string>

namespace cucumber_cpp
{
    void Rtrim(std::string& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                    {
                        return std::isspace(ch) == 0;
                    })
                    .base(),
            s.end());
    }
}
