#ifndef TEST_HELPER_REPORTFORWARDERMOCK_HPP
#define TEST_HELPER_REPORTFORWARDERMOCK_HPP

#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include <cstddef>
#include <filesystem>
#include <gmock/gmock.h>
#include <optional>
#include <string>

namespace cucumber_cpp::library::report::test_helper
{
    struct ReportForwarderMock : report::ReportForwarderImpl
    {
        using ReportForwarderImpl::ReportForwarderImpl;
        virtual ~ReportForwarderMock() = default;

        MOCK_METHOD(void, StepMissing, (const std::string& stepText), (override));
        MOCK_METHOD(void, StepAmbiguous, (const std::string& stepText, const engine::StepInfo& stepInfo), (override));

        MOCK_METHOD(void, Failure, (const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column), (override));
        MOCK_METHOD(void, Error, (const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column), (override));
    };
}

#endif
