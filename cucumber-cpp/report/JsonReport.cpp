#include "cucumber-cpp/report/JsonReport.hpp"
#include "nlohmann/json.hpp"
#include <fstream>

namespace cucumber_cpp::report
{
    void JsonReport::GenerateReport(const nlohmann::json& json)
    {
        std::ofstream("out.json") << json.dump(2) << "\n";
    }
}
