#ifndef REPORT_JSONREPORT_HPP
#define REPORT_JSONREPORT_HPP

#include "cucumber-cpp/report/Report.hpp"

namespace cucumber_cpp::report
{
    struct JsonReport : Report
    {
        void GenerateReport(const nlohmann::json& json) override;
    };
}

#endif
