#include "cucumber_cpp/library/plugin/PluginExport.hpp"
#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"

extern "C" CCR_EXPORT void ccr_register(cucumber_cpp::library::plugin::PluginHostContext* context)
{
    if (context == nullptr || context->registration == nullptr)
        return;

    auto& host = *static_cast<cucumber_cpp::library::support::DefinitionRegistration*>(context->registration);
    host.RegisterPlugin(cucumber_cpp::library::support::DefinitionRegistration::Instance());

    if (context->stopwatch != nullptr)
        cucumber_cpp::library::util::Stopwatch::SetInstance(*static_cast<cucumber_cpp::library::util::Stopwatch*>(context->stopwatch));

    if (context->timestampGenerator != nullptr)
        cucumber_cpp::library::util::TimestampGenerator::SetInstance(*static_cast<cucumber_cpp::library::util::TimestampGenerator*>(context->timestampGenerator));
}
