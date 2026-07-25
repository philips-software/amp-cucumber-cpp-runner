#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
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

    if (context->converterMap != nullptr)
    {
        auto* hostMap = static_cast<cucumber_cpp::library::cucumber_expression::ConverterMap*>(context->converterMap);
        auto& local = cucumber_cpp::library::cucumber_expression::ConverterRegistry::LocalInstance();

        // On Linux (ELF unique symbols), host and plugin share the same
        // static map, so migration is unnecessary and clearing would
        // destroy the host's converters. Only migrate on platforms where
        // they are separate (e.g. Windows DLLs).
        if (&local != hostMap)
        {
            // Migrate any converters this plugin already registered into its
            // local map (built-ins constructed during plugin startup) into the
            // host-owned map, without overwriting existing host entries.
            for (auto& [name, fn] : local)
                hostMap->try_emplace(name, std::move(fn));
            local.clear();

            // Redirect all future registrations and lookups in this plugin to the
            // host-owned converter map so host and plugin share the same storage.
            cucumber_cpp::library::cucumber_expression::ConverterRegistry::SetInstance(hostMap);
        }
    }
}
