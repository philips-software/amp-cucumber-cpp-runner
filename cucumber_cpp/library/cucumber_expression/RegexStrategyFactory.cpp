#include "cucumber_cpp/library/cucumber_expression/RegexStrategyFactory.hpp"
#include "cucumber_cpp/library/cucumber_expression/StdRegexStrategy.hpp"
#ifdef CCR_HAS_RE2
#include "cucumber_cpp/library/cucumber_expression/Re2RegexStrategy.hpp"
#include <stdexcept>
#endif
#include <memory>
#include <string_view>

namespace cucumber_cpp::library::cucumber_expression
{
    std::unique_ptr<RegexStrategy> CreateRegexStrategy(std::string_view pattern)
    {
#ifdef CCR_HAS_RE2
        try
        {
            return std::make_unique<Re2RegexStrategy>(pattern);
        }
        catch (const std::invalid_argument&)
        {
            return std::make_unique<StdRegexStrategy>(pattern);
        }
#else
        return std::make_unique<StdRegexStrategy>(pattern);
#endif
    }
}
