#include "cucumber_cpp/library/plugin/PluginExport.hpp"
#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"

extern "C" CCR_EXPORT void ccr_register(void* hostRegistration)
{
    if (hostRegistration == nullptr)
        return;

    auto& host = *static_cast<cucumber_cpp::library::support::DefinitionRegistration*>(hostRegistration);
    cucumber_cpp::library::support::DefinitionRegistration::Instance().MergeInto(host);
}
