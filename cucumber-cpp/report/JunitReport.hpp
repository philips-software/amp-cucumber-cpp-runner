#ifndef REPORT_JUNITREPORT_HPP
#define REPORT_JUNITREPORT_HPP

#include "cucumber-cpp/report/Report.hpp"

namespace cucumber_cpp::report
{
    struct JunitReport : Report
    {
        void GenerateReport(const nlohmann::json& json) override;
    };
}

#endif
