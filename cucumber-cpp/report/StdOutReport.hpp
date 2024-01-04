#ifndef REPORT_STDOUTREPORT_HPP
#define REPORT_STDOUTREPORT_HPP

#include "cucumber-cpp/report/Report.hpp"

namespace cucumber_cpp::report
{
    struct StdOutReport : Report
    {
        void GenerateReport(const nlohmann::json& json) override;
    };
}

#endif
