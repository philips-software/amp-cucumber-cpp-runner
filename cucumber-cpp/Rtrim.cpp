#include "cucumber-cpp/Rtrim.hpp"
#include <algorithm>

namespace cucumber_cpp
{
    void Rtrim(std::string& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                    {
                        return !std::isspace(ch);
                    })
                    .base(),
            s.end());
    }
}
