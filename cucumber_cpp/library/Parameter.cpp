#include "cucumber_cpp/CucumberCpp.hpp"
#include <compare>
#include <set>
#include <tuple>

namespace cucumber_cpp::library
{
    std::strong_ordering ParameterEntry::operator<=>(const ParameterEntry& other) const
    {
        return std::tie(params.name, params.regex) <=> std::tie(other.params.name, other.params.regex);
    }

    ParameterRegistration& ParameterRegistration::Instance()
    {
        static ParameterRegistration instance;
        return instance;
    }

    const std::set<ParameterEntry>& ParameterRegistration::GetRegisteredParameters() const
    {
        return customParameters;
    }

}
