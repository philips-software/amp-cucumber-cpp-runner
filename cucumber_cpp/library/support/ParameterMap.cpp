#include "cucumber_cpp/library/support/ParameterMap.hpp"
#include <algorithm>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::support
{
    namespace
    {
        constexpr std::string_view specialCharacters{ R"(\^$.|?*+()[]{})" };

        void ValidateKeys(const std::vector<std::string_view>& keys)
        {
            if (keys.empty())
                throw std::invalid_argument{ "PARAMETER_MAP requires at least one key" };

            std::set<std::string_view> uniqueKeys;
            for (const auto key : keys)
            {
                if (key.empty())
                    throw std::invalid_argument{ "PARAMETER_MAP keys must not be empty" };

                if (!uniqueKeys.insert(key).second)
                    throw std::invalid_argument{ "PARAMETER_MAP contains duplicate key: " + std::string{ key } };
            }
        }
    }

    std::string EscapeRegex(std::string_view text)
    {
        std::string result;
        result.reserve(text.size() * 2);

        for (const auto chr : text)
        {
            if (specialCharacters.find(chr) != std::string_view::npos)
                result += '\\';
            result += chr;
        }

        return result;
    }

    std::string KeysToRegex(std::vector<std::string_view> keys)
    {
        ValidateKeys(keys);

        std::ranges::stable_sort(keys, [](std::string_view lhs, std::string_view rhs)
            {
                return lhs.size() > rhs.size();
            });

        std::string result;
        for (const auto key : keys)
        {
            result += result.empty() ? '(' : '|';
            result += EscapeRegex(key);
        }
        result += ')';

        return result;
    }
}
