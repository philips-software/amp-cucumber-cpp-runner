#ifndef LIBRARY_PARAMETER_HPP
#define LIBRARY_PARAMETER_HPP

// IWYU pragma: private, include "cucumber_cpp/CucumberCpp.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include "cucumber/messages/group.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include <compare>
#include <cstddef>
#include <set>
#include <source_location>
#include <string>

namespace cucumber_cpp::library
{
    struct ParameterEntryParams
    {
        std::string name;
        std::string regex;
        bool useForSnippets;
    };

    struct ParameterEntry
    {
        ParameterEntryParams params;

        std::size_t localId;

        std::source_location location;

        std::strong_ordering operator<=>(const ParameterEntry& other) const;
    };

    struct ParameterRegistration
    {
    private:
        ParameterRegistration() = default;

    public:
        static ParameterRegistration& Instance();

        template<class Transformer, class TReturn>
        std::size_t Register(ParameterEntryParams params, std::source_location location = std::source_location::current());

        const std::set<ParameterEntry>& GetRegisteredParameters() const;

    private:
        std::set<ParameterEntry> customParameters;
    };

    ////////////////////
    // Implementation //
    ////////////////////

    template<class Transformer, class TReturn>
    std::size_t ParameterRegistration::Register(ParameterEntryParams params, std::source_location location)
    {
        customParameters.emplace(params, customParameters.size() + 1, location);

        cucumber_expression::ConverterTypeMap<TReturn>::Instance()[params.name] = Transformer::Transform;

        return customParameters.size();
    }
}

#define PARAMETER_STRUCT CONCAT(ParameterImpl, __LINE__)

#define PARAMETER(Type, ...)                                                                                                                             \
    namespace                                                                                                                                            \
    {                                                                                                                                                    \
        struct PARAMETER_STRUCT                                                                                                                          \
        {                                                                                                                                                \
            static Type Transform(const cucumber::messages::group& group);                                                                               \
            static const std::size_t ID;                                                                                                                 \
        };                                                                                                                                               \
    }                                                                                                                                                    \
    const std::size_t PARAMETER_STRUCT::ID = cucumber_cpp::library::ParameterRegistration::Instance().Register<PARAMETER_STRUCT, Type>({ __VA_ARGS__ }); \
    Type PARAMETER_STRUCT::Transform(const cucumber::messages::group& group)

#endif
