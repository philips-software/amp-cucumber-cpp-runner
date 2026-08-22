#include "cucumber_cpp/library/plugin/PluginExport.hpp"
#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

extern "C" void ccr_register(cucumber_cpp::library::plugin::PluginHostContext* context);

namespace cucumber_cpp::library::plugin
{
    TEST(PluginRegister, ignores_null_context)
    {
        EXPECT_NO_THROW(ccr_register(nullptr));
    }

    TEST(PluginRegister, ignores_abi_version_mismatch)
    {
        PluginHostContext context{
            .abiVersion = static_cast<std::uint32_t>(pluginAbiVersion + 1),
            .structSize = static_cast<std::uint32_t>(sizeof(PluginHostContext)),
        };

        EXPECT_NO_THROW(ccr_register(&context));
    }

    TEST(PluginRegister, ignores_undersized_context)
    {
        PluginHostContext context{
            .abiVersion = pluginAbiVersion,
            .structSize = static_cast<std::uint32_t>(sizeof(PluginHostContext) - 1),
        };

        EXPECT_NO_THROW(ccr_register(&context));
    }

    TEST(PluginRegister, ignores_null_registration)
    {
        PluginHostContext context{
            .abiVersion = pluginAbiVersion,
            .structSize = static_cast<std::uint32_t>(sizeof(PluginHostContext)),
            .registration = nullptr,
        };

        EXPECT_NO_THROW(ccr_register(&context));
    }
}
